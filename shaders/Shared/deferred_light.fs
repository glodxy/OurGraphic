layout(binding = 0) uniform sampler2D gBufferA;
layout(binding = 1) uniform sampler2D gBufferB;
layout(binding = 2) uniform sampler2D gBufferC;
layout(binding = 3) uniform sampler2D gBufferD;
layout(binding = 4) uniform sampler2D gBufferE;

layout(location = 0) out vec4 fragColor;
void main() {
    fragColor = vec4(0, 1, 0, 1);
}