void main() {
  vec4 w_pos = GetWorldFromModelMatrix()*mesh_position;
  vertex_worldPosition = w_pos.xyz;
  vertex_worldNormal = (GetWorldFromModelMatrix()*mesh_tangents).xyz;
  vec4 clip_pos = GetClipFromWorldMatrix() * w_pos;
  clip_pos.xyz /= clip_pos.w;
  clip_pos.w = 1.f;
  gl_Position = clip_pos;
  vertex_position = gl_Position;
}