void main() {
    MaterialVertexInputs material;
    InitMaterialVertex(material);

#ifdef HAS_ATTRIBUTE_TANGENTS
    ToTangentFrame(mesh_tangents, material.worldNormal, vertex_worldTangent.xyz);
    mat3 rotate = mat3(GetWorldFromModelMatrix());
    material.worldNormal = rotate * material.worldNormal;
    vertex_worldTangent.xyz = rotate * vertex_worldTangent.xyz;
    vertex_worldTangent.w = mesh_tangents.w;
#endif

  // 材质自定义处理
    HandleVertex(material);

#if defined(HAS_ATTRIBUTE_COLOR)
    vertex_color = material.color;
#endif
#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material.uv1;
#endif

#ifdef HAS_ATTRIBUTE_TANGENTS
    vertex_worldNormal = material.worldNormal;
#endif
    // 材质自定义输出
    HandleCustomOutput(material);

    vec4 clip_pos = material.clipPosition;
    clip_pos.xyz = clip_pos.xyz / clip_pos.w;
    clip_pos.w = 1;
    gl_Position = clip_pos;
}