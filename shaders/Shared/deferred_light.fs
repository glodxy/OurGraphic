layout(binding = 0, set = 1) uniform sampler2D gBufferA;
layout(binding = 1, set = 1) uniform sampler2D gBufferB;
layout(binding = 2, set = 1) uniform sampler2D gBufferC;
layout(binding = 3, set = 1) uniform sampler2D gBufferD;
layout(binding = 4, set = 1) uniform sampler2D gBufferE;

layout(location = 0) in vec2 vertex_uv;
layout(location = 1) in vec2 normal_uv;

layout(location = 0) out vec4 fragColor;
void main() {
    vec4 a = texture(gBufferA, normal_uv);
    vec4 b = texture(gBufferB, normal_uv);
    vec4 c = texture(gBufferC, normal_uv);
    vec4 d = texture(gBufferD, normal_uv);
    vec4 e = texture(gBufferE, normal_uv);
    GBufferData data = DecodeGBuffer(a, b, c, d, e, 1.0);
//
    uint lightCount = GetDynamicLightCount();

    vec3 v = -(data.worldPosition.xyz - frameUniform.cameraPosition);
    vec3 n = normalize(data.worldNormal);
    float roughness = data.roughness;
    vec3 diffuse = data.diffuseColor;
    vec3 f0 = data.specularColor;

    vec3 color = vec3(0);
    if(lightCount > 0 ) {
        for(uint i = 0; i < 2; ++i) {
            LightData light = DecodeLightData(GetDynamicLight(i));
            vec3 l = -(data.worldPosition.xyz - light.position);
            if (dot(l, n) < 0) {
                continue;
            }
            float light_dis = length(data.worldPosition.xyz - light.position);
            vec3 brdf = CalcBRDF(n, l, v, roughness, f0, diffuse);
            float intensity = light.intensity / (light_dis * light_dis);
            color += (brdf * light.color.xyz * intensity);
        }
    }

    //vec4 rate = CalcSingleLight(light, data.worldNormal, frameUniform.cameraPosition, data.worldPosition.xyz);
    fragColor = vec4(color, 1);
}