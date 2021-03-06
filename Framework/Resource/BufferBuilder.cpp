//
// Created by Glodxy on 2021/10/14.
//

#include "BufferBuilder.h"


namespace our_graph {
float BufferBuilder::window_rect[2] = {
    .0f, .0f
};

VertexBuffer *BufferBuilder::BuildDefaultVertex(Driver* driver) {
  VertexBuffer* buffer =
      VertexBuffer::Builder(driver)
      .VertexCount(36)
      .BufferCount(2)
      .Attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT3, 0, 0)
      .Attribute(VertexAttribute::TANGENTS, 1, ElementType::FLOAT3, 0, 0)
      .Build();
  BufferDescriptor vertex(default_soft_vertex, sizeof(float) * 36 * 3);
  BufferDescriptor normal(default_soft_normal, sizeof(float) * 36 * 3);
  buffer->SetBufferAt(0, std::move(vertex), 0);
  buffer->SetBufferAt(1, std::move(normal), 0);
  return buffer;
}

IndexBuffer *BufferBuilder::BuildDefaultIndex(Driver *driver) {
  IndexBuffer* buffer =
      IndexBuffer::Builder(driver)
      .IndexCount(36)
      .Build();
  BufferDescriptor index(default_index, sizeof(uint32_t) * 36);
  buffer->SetBuffer(std::move(index));
  return buffer;
}

BufferObject *BufferBuilder::BuildDefaultQuadUniformBuffer(
    Driver *driver,
    float width,
    float height) {
  BufferObject* buffer =
      BufferObject::Builder(driver)
          .Size(sizeof(float) * 2)
          .Build();
  window_rect[0] = width;
  window_rect[1] = height;
  BufferDescriptor resolution(window_rect, sizeof(float) * 2);
  buffer->SetBuffer(std::move(resolution));
  return buffer;
}

}  // namespace our_graph