//
// Created by Glodxy on 2021/9/23.
//

#include "VulkanBaseBufferView.h"
//VulkanVertexDescription//////////////////////////
our_graph::VulkanVertexDescription our_graph::VulkanVertexDescription::Builder::Copy(
    const VulkanVertexDescription &r) {
  VulkanVertexDescription description;
  description.bind_description_ = r.bind_description_;
  description.attribute_descriptions_ = r.attribute_descriptions_;
  return description;
}

bool our_graph::VulkanVertexDescription::Builder::Build(void* create_info, VulkanVertexDescription &description) {
  //todo
  return true;
}

//VulkanBaseBufferView//////////////////////////
