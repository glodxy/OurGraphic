//
// Created by Glodxy on 2021/10/14.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_VERTEXBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_VERTEXBUFFER_H_
#include <bitset>
#include "base/BuilderBase.h"
#include "Backend/include/Handle.h"
#include "Backend/include/BufferDescriptor.h"
#include "Backend/include/DriverEnum.h"
#include "Backend/include/Driver.h"
#include "include_internal/ResourceBase.h"
namespace our_graph {
class VertexBuffer : public ResourceBase {
  friend class ResourceAllocator;
  struct Detail;

  struct AttributeData : public Attribute {
    AttributeData() : Attribute{.type = ElementType::FLOAT4} {

    }
  };

 public:
  using AttributeType = ElementType;

  class Builder : public BuilderBase<Detail> {
    friend class Detail;
    friend class VertexBuffer;
   public:
    Builder(DriverApi* driver) noexcept;
    Builder(const Builder& rhs) noexcept;
    Builder(Builder&& rhs) noexcept;
    ~Builder() noexcept;
    Builder& operator=(const Builder& rhs) noexcept;
    Builder& operator=(Builder&& rhs) noexcept;

    Builder& BufferCount(uint8_t buffer_count) noexcept;

    /**
     * 每个buffer的顶点数目
     * */
    Builder& VertexCount(uint32_t vertex_count) noexcept;

    /**
     * 是否启用Object模式，如果启用则需要传递Handle而非具体的数据
     * */
    Builder& EnableBufferObjects(bool enabled = true) noexcept;

    Builder& Attribute(VertexAttribute attribute, uint8_t buffer_index,
                       AttributeType attribute_type, uint32_t byte_offset = 0,
                       uint8_t byte_stride = 0) noexcept;


    VertexBuffer* Build();
  };

  size_t GetVertexCount() const noexcept;

  void SetBufferAt(uint8_t buffer_index, BufferDescriptor&& buffer,
                   uint32_t byte_offset = 0);

  void SetBufferObjectAt(uint8_t buffer_index, BufferObjectHandle buffer);

  VertexBufferHandle GetHandle() const {return handle_;}

  void Destroy() override;

  ~VertexBuffer() override = default;
 protected:
  // 构造函数
  VertexBuffer(const Builder& builder);
 private:
  VertexBufferHandle handle_;
  std::array<AttributeData ,MAX_VERTEX_ATTRIBUTE_COUNT> attributes_;
  std::array<BufferObjectHandle , MAX_VERTEX_BUFFER_COUNT> buffers_;
  std::bitset<32> declared_attributes_; // 标识有哪些attribute已被声明
  uint32_t vertex_count_ = 0;
  uint8_t buffer_count_ = 0;
  const bool enable_buffer_objects_ = false;
  DriverApi* driver_;
};
}

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_VERTEXBUFFER_H_
