layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vertex_uv01, 1.f, 1.f);
}