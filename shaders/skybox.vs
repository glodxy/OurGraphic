void HandleCustomOutput(MaterialVertexInputs inputs) {
    variable_direction = mesh_position.xyz;
}

void HandleVertex(inout MaterialVertexInputs inputs) {
    mat3 v = mat3(GetViewFromWorldMatrix());
    vec3 pos = mesh_position.xyz;
    vec4 view_pos = vec4(v*pos, 1.0);
    inputs.clipPosition = GetClipFromViewMatrix() * view_pos;
    float real_z = inputs.clipPosition.z / inputs.clipPosition.w;
    if (real_z >=0 && real_z <= 1) {
        inputs.clipPosition.z = inputs.clipPosition.w;
    }
}
