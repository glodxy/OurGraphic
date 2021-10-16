//
// Created by Glodxy on 2021/10/14.
//

#include "BufferBuilder.h"


namespace our_graph {

VertexBuffer *BufferBuilder::BuildDefaultVertex(DriverApi* driver) {
  VertexBuffer* buffer =
      VertexBuffer::Builder(driver)
      .VertexCount(4)
      .BufferCount(1)
      .Attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT4, 0, 0)
      .Build();
  BufferDescriptor vertex(default_vertex, sizeof(float) * 16);
  buffer->SetBufferAt(0, std::move(vertex), 0);
  return buffer;
}

IndexBuffer *BufferBuilder::BuildDefaultIndex(DriverApi *driver) {
  IndexBuffer* buffer =
      IndexBuffer::Builder(driver)
      .IndexCount(6)
      .Build();
  BufferDescriptor index(default_index, sizeof(uint32_t) * 6);
  buffer->SetBuffer(std::move(index));
  return buffer;
}

}  // namespace our_graph