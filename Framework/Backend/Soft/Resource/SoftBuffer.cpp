//
// Created by Glodxy on 2021/11/10.
//

#include "SoftBuffer.h"
#include "Backend/include/DriverApi.h"
namespace our_graph {
SoftBuffer::SoftBuffer(uint32_t bytes) : IBufferObject(bytes){
  buffer = ::malloc(bytes);
}

SoftBuffer::SoftBuffer(ElementType type, uint32_t cnt) : elem_type(type) {
  byte_cnt_ = DriverApi::GetElementTypeSize(type) * cnt;
  buffer = ::malloc(byte_cnt_);
}
SoftBuffer::~SoftBuffer() {
  if (buffer) {
    ::free(buffer);
  }
}

SoftVertexBuffer::SoftVertexBuffer(uint8_t buffer_cnt,
                                   uint8_t attr_cnt,
                                   uint32_t element_cnt,
                                   const AttributeArray &attributes) :
    IVertexBuffer(buffer_cnt, attr_cnt, element_cnt, attributes),
    buffers_(buffer_cnt, nullptr) {

}

SoftIndexBuffer::SoftIndexBuffer(ElementType element_type, uint32_t index_cnt)
  : IIndexBuffer(DriverApi::GetElementTypeSize(element_type), index_cnt) {
  buffer_ = new SoftBuffer(element_type, index_cnt);
}

SoftIndexBuffer::~SoftIndexBuffer() {
  delete buffer_;
}

}