#include "input.vs"
mat4 GetModelToWorldMatrix() {
  return MeshUniform.model_to_world;
}

mat4 GetWorldToViewMatrix() {
  return FrameUniform.world_to_view;
}

mat4 GetViewToClipMatrix() {
  return FrameUniform.view_to_clip;
}

vec4 ComputePosition(vec4 position) {
  mat4 m = GetModelToWorldMatrix();
  mat4 v = GetWorldToViewMatrix();
  mat4 p = GetViewToClipMatrix();
  return p * v * m * position;
}