void main() {
    MaterialInputs material;
    InitMaterial(material);

    // 材质定义的处理
    HandleMaterial(material);

#if defined(RENDER_PATH_DEFERRED)
    GBufferData data;
    WriteMaterialToGBuffer(material, data);
    data.worldNormal = material.normal;
    data.worldPosition = vec4(vertex_worldPosition, 1.f);
    vec4 a,b,c,d,e;
    EncodeGBuffer(data, a, b, c, d, e);
    gBufferA = a;
    gBufferB = b;
    gBufferC = c;
    gBufferD = d;
    gBufferE = e;
#elif defined(RENDER_PATH_FORWARD)
    outColor = EvaluateMaterial(material);
#endif
}