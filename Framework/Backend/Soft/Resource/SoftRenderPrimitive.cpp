//
// Created by Glodxy on 2021/11/10.
//

#include "SoftRenderPrimitive.h"
#include "SoftBuffer.h"
namespace our_graph {

SoftRenderPrimitive::SoftRenderPrimitive() = default;

void SoftRenderPrimitive::BindVertex(SoftVertexBuffer *vertex_buffer) {
  buffer_ = vertex_buffer;
}

size_t SoftRenderPrimitive::GetVertexCnt() const {
  return buffer_->vertex_cnt_;
}

// todo：目前默认取第一个buffer
void *const SoftRenderPrimitive::GetVertexData() const {
  return buffer_->buffers_[0]->buffer;
}

SoftVertexBuffer * SoftRenderPrimitive::GetVertexBuffer() {
  return buffer_;
}
}