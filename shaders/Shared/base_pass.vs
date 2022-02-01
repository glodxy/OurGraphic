void main() {
    MaterialVertexInputs material_;
    InitMaterialVertex(material);

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
    HandleCustomOutput();


    gl_Position = GetClipFromWorldMatrix() * material.worldPosition;
}