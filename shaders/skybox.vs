void HandleCustomOutput(MaterialVertexInputs inputs) {
    variable_direction = mesh_position.xyz;
}

void HandleVertex(inout MaterialVertexInputs inputs) {
    mat4 v = mat4(mat3(GetViewFromWorldMatrix()));
    vec4 pos = vec4(mesh_position.xyz, 1);
    vec4 view_pos = v*pos;
    inputs.clipPosition = GetClipFromViewMatrix()*view_pos;
    inputs.clipPosition.z = inputs.clipPosition.w;
}
