void HandleMaterial(inout MaterialInputs inputs) {
    inputs.metallic = texture(materialParams_metallicSampler, vertex_uv01.xy).x;
    inputs.metallic = texture(materialParams_roughnessSampler, vertex_uv01.xy).x;
}