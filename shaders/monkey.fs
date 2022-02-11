void HandleMaterial(inout MaterialInputs inputs) {
    inputs.metallic = texture(materialParams_metallicSampler, vertex_uv01.xy).x;
    inputs.roughness = texture(materialParams_roughnessSampler, vertex_uv01.xy).x;
    inputs.baseColor.xyz = vec3(texture(materialParams_metallicSampler, vertex_uv01).x);
    inputs.baseColor.w = 1.0f;
    inputs.shadingModel = 1;
}