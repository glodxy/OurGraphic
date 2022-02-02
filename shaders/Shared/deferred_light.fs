layout(binding = 0) uniform sampler2D gBufferA;
layout(binding = 1) uniform sampler2D gBufferB;
layout(binding = 2) uniform sampler2D gBufferC;
layout(binding = 3) uniform sampler2D gBufferD;
layout(binding = 4) uniform sampler2D gBufferE;

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


    fragColor = vec4(1, 0, 0, 1);
}