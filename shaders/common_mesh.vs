vec4 GetPosition() {
  vec4 pos = vec4(position, 1.0);
  return pos;
}

mat4 GetModelToWorldMatrix() {
  return MeshUniform.model_to_world;
}

mat4 GetWorldToViewMatrix() {
  return FrameUniform.world_to_view;
}

mat4 GetViewToClipMatrix() {
  return FrameUniform.view_to_clip;
}

vec4 ComputePosition() {
  mat4 m = GetModelToWorldMatrix();
  mat4 v = GetWorldToViewMatrix();
  mat4 p = GetViewToClipMatrix();
  vec4 position = GetPosition();
  return p * v * m * position;
}