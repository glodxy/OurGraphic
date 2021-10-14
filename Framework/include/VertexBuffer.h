//
// Created by Glodxy on 2021/10/14.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_VERTEXBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_VERTEXBUFFER_H_
#include "base/BuilderBase.h"
#include "Backend/include/Handle.h"
#include "Backend/include/BufferDescriptor.h"
#include "Backend/include/DriverEnum.h"
#include "Backend/include/Driver.h"

namespace our_graph {
class VertexBuffer {
  struct Detail;

 public:
  using AttributeType = ElementType;

  class Builder : public BuilderBase<Detail> {
    friend class Detail;
   public:
    Builder() noexcept;
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

  void SetBufferAt(DriverApi* driver, uint8_t buffer_index, BufferDescriptor&& buffer,
                   uint32_t byte_offset = 0);

  void SetBufferObjectAt(DriverApi* driver, uint8_t buffer_index, BufferObjectHandle buffer);
};
}

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_VERTEXBUFFER_H_
