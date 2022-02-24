void HandleMaterial(inout MaterialInputs inputs) {
    vec3 pos = variable_direction;
    inputs.baseColor = texture(materialParams_sky, variable_direction);
    //inputs.baseColor = vec4(1,1,1, 1.0);
}