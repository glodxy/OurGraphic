layout(location = 0) out vec4 outColor;

vec3 CalcDirectionLight(vec3 light_color, vec3 normal, vec3 view_dir) {
    vec3 material = vec3(1.0f, 1.0f, 1.0f);

    float power = 0.4f;
    vec3 light_dir = vec3(0, 0, 1);
    vec3 ambient = vec3(0.2f, 0.2f, 0.2f);
    vec3 diffuse = vec3(1.f,1.f,1.f) * power * max(0.f, dot(-light_dir, normal));
    float specular_recv = pow(max(0.f, dot((-(normalize(light_dir + view_dir))), normal)), 256);
    vec3 specular = light_color * power * specular_recv;

    vec3 color = material * (ambient + diffuse + specular);
    return color;
}

void main() {

    vec3 material = vec3(1.0f, 1.0f, 1.0f);
    vec3 normal = normalize(vertex_worldNormal);
    vec3 light_pos = vec3(0.f, 0.f, 2.f);
    vec3 view_pos = vec3(0.f, 0.f, 1.f);
    vec3 light_dir = vertex_worldPosition - light_pos;
    vec3 view_dir = normalize(vertex_worldPosition - view_pos);
    vec3 light_color = vec3(1.f, 1.f, 1.f);
    float power = 1.f / pow(length(light_dir), 2);
    //float power = 10;
    light_dir = normalize(light_dir);
    vec3 ambient = vec3(0.2f, 0.2f, 0.2f);
    vec3 diffuse = vec3(.6f,.6f,.6f) * power * max(0.f, dot(-light_dir, normal));
    float specular_recv = pow(max(0.f, dot((-(normalize(light_dir + view_dir))), normal)), 64);
    vec3 specular = light_color * power * specular_recv;

    vec3 color = material * (ambient + diffuse + specular);
    //color = clamp(color, 0.f, 1.f);

    outColor = vec4(color, 1.0f);
}