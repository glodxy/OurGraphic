void HandleMaterial(inout MaterialInputs inputs) {
    inputs.metallic = texture(materialParams_metallicSampler, vertex_uv01.xy).x;
    inputs.roughness = texture(materialParams_roughnessSampler, vertex_uv01.xy).x;
    inputs.baseColor = texture(materialParams_albedoSampler, vertex_uv01);

    vec3 tangent_normal = texture(materialParams_normalSampler, vertex_uv01).xyz * 2.0 - 1.0;
    vec3 normal = GetTangentToWorldMatrix() * tangent_normal;


    inputs.normal = normal;
}