//
// Created by Glodxy on 2021/9/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
#include "../include_internal/IResource.h"
#include "VulkanBaseBuffer.h"
#include "VulkanBaseBufferView.h"

namespace our_graph {

template<class Description = EmptyDescription>
class VulkanBuffer :
    public IResource<VulkanBaseBuffer, VulkanBaseBufferView<Description>> {
  using ViewType = VulkanBaseBufferView<Description>;
 public:
  explicit VulkanBuffer(const std::string& name,
                        VkDevice device,
                        VkBufferCreateInfo buffer_create_info,
                        Description description,
                        uint64_t memory_flag_bits = 0) :
                        device_(device),
                        name_(name){
    this->buffer_ = std::make_shared<VulkanBaseBuffer>(name_, device_,
                                                 buffer_create_info, memory_flag_bits);
    this->descriptor_ = std::make_shared<ViewType>(device_, description);
  }

  explicit VulkanBuffer(const std::string& name,
                        VkDevice device,
                        VkBufferCreateInfo buffer_create_info,
                        VkBufferViewCreateInfo view_create_info,
                        Description description,
                        uint64_t memory_flag_bits = 0) :
                        device_(device),
                        name_(name){
    this->buffer_ = std::make_shared<VulkanBaseBuffer>(name_, device_,
                                                       buffer_create_info, memory_flag_bits);

    VkBuffer* real_buffer = (VkBuffer*)(this->buffer_)->GetInstance();
    view_create_info.buffer = *real_buffer;
    this->descriptor_ = std::make_shared<ViewType>(device_, view_create_info,
                                                               description);
  }

  ~VulkanBuffer() override {
    this->descriptor_->Destroy();
    this->buffer_ = nullptr;
    LOG_INFO("VulkanBuffer", "Destroy Buffer:{}", name_);
  }

  std::shared_ptr<IDescriptor> GetView() override {
    return this->descriptor_;
  }

  std::shared_ptr<IBuffer> GetBuffer() override {
    return this->buffer_;
  }

 protected:

  void Create() override {

  }


  void Destroy() override {

  }
 protected:
  VkDevice device_;
  std::string name_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANBUFFER_H_
