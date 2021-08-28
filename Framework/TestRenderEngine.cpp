//
// Created by Glodxy on 2021/8/28.
//

#include "TestRenderEngine.h"

#include "Backend/Vulkan/VulkanInstance.h"

namespace our_graph {
void TestRenderEngine::Init() {
  render_instance_ = std::make_shared<VulkanInstance>();
  render_instance_->CreateInstance();
}

void TestRenderEngine::Destroy() {

}

void TestRenderEngine::End() {

}

void TestRenderEngine::Start() {

}


void TestRenderEngine::AfterRender() {

}

void TestRenderEngine::BeforeRender() {

}

void TestRenderEngine::Render() {

}

}  // namespace our_graph