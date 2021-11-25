#version 450
layout(location = 0) in vec3 position;
layout(location = 8) in vec3 normal;

layout(location = 8) out vec3 world_normal;
layout(location = 0) out vec3 world_position;

layout(binding = 0) uniform ObjUniform {
    mat4 model_to_world;
} objUniform;

layout(binding = 1) uniform FrameUniform {
    mat4 world_to_view;
    mat4 view_to_clip;
} frameUniform;

mat4 GetModelToWorldMatrix() {
    return objUniform.model_to_world;
}

mat4 GetWorldToViewMatrix() {
    return frameUniform.world_to_view;
}

mat4 GetViewToClipMatrix() {
  return frameUniform.view_to_clip;
}

vec4 ComputePosition(vec4 position) {
  mat4 m = GetModelToWorldMatrix();
  mat4 v = GetWorldToViewMatrix();
  mat4 p = GetViewToClipMatrix();
  return p * v * m * position;
}



void main() {
    mat4 tmp = mat4(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0.5, 0, 0, 1);
    world_position = position;
    world_normal = normal;
    vec4 clip_pos = ComputePosition(vec4(position, 1.0));
    gl_Position = clip_pos;
}