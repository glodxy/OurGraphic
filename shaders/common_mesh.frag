#version 450
layout(location = 0) in vec3 world_position;
layout(location = 8) in vec3 world_normal;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 material = vec3(1.0f, 1.0f, 1.0f);
    vec3 normal = normalize(world_normal);
    vec3 light_pos = vec3(0.f, 0.f, 2.f);
    vec3 view_pos = vec3(0.f, 0.f, 1.f);
    vec3 light_dir = world_position - light_pos;
    vec3 view_dir = normalize(world_position - view_pos);
    vec3 light_color = vec3(1.f, 1.f, 1.f);
    float power = 1.f / pow(length(light_dir), 2);
    light_dir = normalize(light_dir);
    vec3 ambient = vec3(0.2f, 0.2f, 0.2f);
    vec3 diffuse = vec3(1.f,1.f,1.f) * power * max(0.f, dot(-light_dir, normal));
    float specular_recv = pow(max(0.f, dot((-(normalize(light_dir + view_dir))), normal)), 256);
    vec3 specular = light_color * power * specular_recv;

    vec3 color = material * (ambient + diffuse);
    color = clamp(color, 0.f, 1.f);
    outColor = vec4(color, 1.0f);
}