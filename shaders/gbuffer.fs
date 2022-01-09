layout(location = 0) out vec4 gBufferA;
layout(location = 0) out vec4 gBufferB;
layout(location = 0) out vec4 gBufferC;
layout(location = 0) out vec4 gBufferD;
layout(location = 0) out vec4 gBufferE;


void main() {
    GBufferData data;
    data.shadingModel = materialParams.shadingModel;
    data.worldNormal = vertex_worldNormal;
    data.worldPosition = vec4(vertex_worldPosition, 1.f);
    vec4 a,b,c,d,e;
    EncodeGBuffer(data, a, b, c, d, e);
    gBufferA = a;
    gBufferB = b;
    gBufferC = c;
    gBufferD = d;
    gBufferE = e;
}