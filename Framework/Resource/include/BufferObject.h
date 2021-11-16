//
// Created by Glodxy on 2021/10/17.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_BUFFEROBJECT_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_BUFFEROBJECT_H_
#include "Backend/include/Driver.h"
#include "Backend/include/DriverEnum.h"
#include "Backend/include/BufferDescriptor.h"
#include "Framework/Resource/base/BuilderBase.h"
#include "Framework/Resource/include_internal/ResourceBase.h"
namespace our_graph {

class BufferObject : public ResourceBase {
  struct Detail;
  friend class ResourceAllocator;
 public:

  class Builder : public BuilderBase<Detail> {
    friend class BufferObject;
   public:
    Builder(Driver* driver) noexcept;
    Builder(const Builder& rhs) noexcept;
    Builder(Builder&& rhs) noexcept;
    ~Builder() noexcept;
    Builder& operator=(const Builder& rhs) noexcept;
    Builder& operator=(Builder&& rhs) noexcept;

    Builder& Size(uint32_t byte_count) noexcept;

    /**
     * Buffer的绑定类型,是uniform还是vertex
     * 默认是uniform
     * */
    Builder& BindingType(BufferObjectBinding bind_type) noexcept;

    BufferObject* Build();
  };

  void SetBuffer(BufferDescriptor&& buffer, uint32_t byte_offset = 0);

  size_t GetByteCount() const {
    return byte_count_;
  }

  void Destroy() override;

  BufferObjectHandle GetHandle() const {
    return handle_;
  }
  BufferObjectBinding GetBindingType() const {
    return binding_type_;
  }
 protected:
  BufferObject(const Builder& builder);

 private:
  BufferObjectHandle handle_;
  uint32_t byte_count_;
  BufferObjectBinding binding_type_;
  Driver* driver_;
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_BUFFEROBJECT_H_
