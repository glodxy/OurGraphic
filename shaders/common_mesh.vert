#version 450
layout(location = 0) in vec3 position;
layout(location = 8) in vec3 normal;

layout(binding = 1) uniform FrameUniform {
    mat4 world_to_view;
    mat4 view_to_clip;
} frameUniform;


mat4 GetWorldToViewMatrix() {
    return frameUniform.world_to_view;
}

mat4 GetViewToClipMatrix() {
  return frameUniform.view_to_clip;
}

vec4 ComputePosition(vec4 position) {
  mat4 v = GetWorldToViewMatrix();
  mat4 p = GetViewToClipMatrix();
  return p * v * position;
}



void main() {
    mat4 tmp = mat4(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0.5, 0, 0, 1);
    gl_Position = ComputePosition(vec4(position, 1.0));
}