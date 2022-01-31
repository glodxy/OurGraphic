layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;
layout(location = 0) out vertex_uv;
void main() {
    vertex_uv = uv * frameUniform.xy;
    vertex_uv.y = frameUniform.y - vertex_uv.y;
    gl_Position = position;
}