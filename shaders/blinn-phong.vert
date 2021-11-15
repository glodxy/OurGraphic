#version 450
#include "input.vs"
#include "common_mesh.vs"


void main() {
    gl_Position = ComputePosition();
}