void HandleCustomOutput(MaterialVertexInputs inputs) {
    variable_direction = mesh_position.xyz;
}

void HandleVertex(inout MaterialVertexInputs inputs) {
    mat4 v = GetViewFromWorldMatrix();
    vec4 pos = vec4(mesh_position.xyz, 1);
    vec4 view_pos = v*pos;
    view_pos.w = 1.0;
    inputs.clipPosition = GetClipFromViewMatrix()*view_pos;
}
