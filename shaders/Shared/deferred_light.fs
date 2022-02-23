layout(location = 0) in vec2 vertex_uv;
layout(location = 1) in vec2 normal_uv;

layout(location = 0) out vec4 fragColor;
void main() {
    vec4 a = texture(gBuffer_A, normal_uv);
    vec4 b = texture(gBuffer_B, normal_uv);
    vec4 c = texture(gBuffer_C, normal_uv);
    vec4 d = texture(gBuffer_D, normal_uv);
    vec4 e = texture(gBuffer_E, normal_uv);
    GBufferData data = DecodeGBuffer(a, b, c, d, e, 1.0);
//
    uint lightCount = GetDynamicLightCount();

    vec3 v = -(data.worldPosition.xyz - frameUniform.cameraPosition);
    vec3 n = normalize(data.worldNormal);
    float roughness = data.roughness;
    vec3 diffuse = data.diffuseColor;
    vec3 f0 = data.specularColor;

    vec3 color = vec3(0);
    uint shading_model = data.shadingModel;
    if (shading_model > 1) {
        if(lightCount > 0 && lightCount < 256) {
            for(uint i = 0; i < lightCount; ++i) {
                LightData light = DecodeLightData(GetDynamicLight(i));
                vec3 l = -(data.worldPosition.xyz - light.position);
                if (dot(l, n) < 0) {
                    continue;
                }
                float light_dis = length(data.worldPosition.xyz - light.position);
                vec3 brdf = CalcBRDF(n, l, v, roughness, f0, diffuse);
                float intensity = light.intensity / (light_dis * light_dis);
                color += (brdf * light.color.xyz * intensity * dot(n, l));
            }
        }
        vec3 ambient = CalcAmbient(v, n, roughness, f0, diffuse);
        color += ambient;
    } else {
        color = data.baseColor.xyz;
    }



    //vec4 rate = CalcSingleLight(light, data.worldNormal, frameUniform.cameraPosition, data.worldPosition.xyz);
    fragColor = vec4(color, 1);
}