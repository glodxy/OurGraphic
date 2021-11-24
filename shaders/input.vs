layout(binding = 0) uniform MeshUniform {
    mat4 model_to_world;
};

layout(binding = 1) uniform FrameUniform {
    mat4 world_to_view;
    mat4 view_to_clip;
}