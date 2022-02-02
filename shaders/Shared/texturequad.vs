
layout(location = 0) in vec2 position;
layout(location = 3) in vec2 uv;
layout(location = 0) out vec2 vertex_uv;
layout(location = 1) out vec2 normal_uv;
void main() {
    normal_uv = uv;
    vertex_uv = uv * frameUniform.resolution.xy;
    vertex_uv.y = frameUniform.resolution.y - vertex_uv.y;
    gl_Position = vec4(position, 0, 1);
}