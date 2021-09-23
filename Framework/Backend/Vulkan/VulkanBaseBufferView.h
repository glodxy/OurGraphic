//
// Created by Glodxy on 2021/9/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBASEBUFFERVIEW_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBASEBUFFERVIEW_H_
#include "../include_internal/IDescriptor.h"
#include "VulkanDef.h"
namespace our_graph {

struct EmptyDescription {
  class Builder {
    static EmptyDescription Copy(const EmptyDescription& r) {
      return EmptyDescription();
    }

    static bool Build(void* create_info, EmptyDescription& description) {
      return true;
    }
  };
};

/**
 * 顶点缓存的描述符
 * */
struct VulkanVertexDescription {
  // 该结构体描述如何从Buffer取数据
  VkVertexInputBindingDescription bind_description_;
  // 该结构体描述shader如何解释取出的数据
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions_;

  /**
   * 该类用于创建对应的Description
   * */
  class Builder {
    static VulkanVertexDescription Copy(const VulkanVertexDescription& r);

    static bool Build(void* create_info, VulkanVertexDescription& description);
  };
};

/**
 * Buffer(通用显存)的View由两部分构成：
 * 1.BufferView，该部分可比为显存的句柄
 * 2.Description，该部分描述了如何暴露给shader
 * */
 template<class Description>
class VulkanBaseBufferView : public IDescriptor {
  using DescriptionBuilder = Description::Builder;
 public:
  explicit VulkanBaseBufferView(VkDevice device,
                                Description description) :
                                device_(device){
    has_view_ = false;
    CreateDescription(description);
  }

  explicit VulkanBaseBufferView(VkDevice device,
                                VkBufferViewCreateInfo create_info,
                                Description description) :
                                device_(device),
                                buffer_view_create_info_(create_info) {
    CreateBufferView();
    CreateDescription(description);
    has_view_ = true;
  }

  void BindBuffer(std::shared_ptr<IBuffer> buffer) override {
    // todo
  }

  void * GetInstance() override {
    return &buffer_view_;
  }

  void * GetDescription() override {
    return &description_;
  }
 protected:

  bool CreateBufferView() {
    VkResult res = vkCreateBufferView(device_, &buffer_view_create_info_,
                                      nullptr, &buffer_view_);
    CHECK_RESULT(res, "VulkanBufferView", "CreateBufferView Failed!");

    return true;
  }

  bool CreateDescription(Description description) {
    description_ = DescriptionBuilder::Copy(description);
//    if (!DescriptionBuilder::Build(description_)) {
//      LOG_ERROR("VulkanBufferView", "CreateDescription Failed!");
//      return false;
//    }
    return true;
  }

  VkDevice device_;

  // 标识是否拥有View
  bool has_view_ {false};
  VkBufferViewCreateInfo buffer_view_create_info_;
  VkBufferView buffer_view_;
  // 描述符
  Description description_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBASEBUFFERVIEW_H_
