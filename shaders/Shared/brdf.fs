#ifdef SHADER_TYPE_FRAGMENT
// 该宏用于限制该文件只在fs使用

#define MEDIUM_FLOAT_MAX 65504.0
#define SaturateMedium(x) min(x, MEDIUM_FLOAT_MAX)

#define PI 3.1415926
// 此处h为半程向量((v + l)/2)
// n为宏观法线
float D_GGX(vec3 h, vec3 n, float roughness) {
    float NoH = clamp(dot(n, h), 0, 1);
    vec3 NxH = cross(n, h);
    float a = NoH * roughness;
    float k = roughness / (dot(NxH, NxH) + a*a);
    float d = k* k * (1.0 / PI);
    return SaturateMedium(d);
}

// float D_GGX(vec3 n, vec3 h, float roughness) {
//     float NoH = clamp(dot(n, h), 0, 1);
//     float a2 = roughness * roughness;
//     float f = (NoH * a2 - NoH) * NoH + 1.0;
//     return a2 / (PI * f * f);
// }

// 高度相关的smith shadow遮蔽项
float V_SmithGGXCorrelated(vec3 n, vec3 v, vec3 l, float roughness) {
    float a2 = roughness * roughness;
    float NoV = clamp(dot(n, v), 0, 1);
    float NoL = clamp(dot(n, l), 0, 1);
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return SaturateMedium(0.5/(GGXV + GGXL));
}

// 菲涅尔项
vec3 F_Schlick(vec3 l, vec3 h, vec3 f0) {
    float u = clamp(dot(l, h), 0, 1);
    float f = pow(1.0 - u, 5);
    return f + f0 * (1.0 - f);
}

// 获取每个光线的概率
float Lambert() {
   return 1/PI;
}

vec3 CalcBRDF(vec3 n, vec3 l, vec3 v, float roughness, vec3 f0, vec3 diffuse) {
    vec3 h = normalize(v + l);

    float D = D_GGX(h, n, roughness);
    float V = V_SmithGGXCorrelated(n, v, l, roughness);
    vec3 F = F_Schlick(l, h, f0);
    vec3 Fr = (D*V)*F;

    vec3 Fd = diffuse * Lambert();

    return Fr + Fd;
}

vec3 CalcAmbient(vec3 v, vec3 n, float roughness, vec3 f0, vec3 diffuseColor) {

  vec3 irradiance = texture(frameSampler_diffuseIrradiance, n).rgb;

  vec3 diffuse = irradiance * diffuseColor;

  vec3 R = reflect(-v, n);
  const float MAX_REFLECTION_LOD = 5.0;

  vec3 prefilteredColor = textureLod(frameSampler_specularPrefilter, R, roughness * MAX_REFLECTION_LOD).rgb;
  vec2 scale_bias = texture(frameSampler_brdfLut, vec2(max(dot(n, v), 0.0), roughness)).rg;
  vec3 specular = prefilteredColor * (f0 * scale_bias.x + scale_bias.y);

  vec3 ambient = diffuse + specular;
  return ambient;
}


#endif