//
// Created by Glodxy on 2021/11/10.
//

#include "SoftBuffer.h"

namespace our_graph {


SoftVertexBuffer::SoftVertexBuffer(uint8_t buffer_cnt,
                                   uint8_t attr_cnt,
                                   uint32_t element_cnt,
                                   const AttributeArray &attributes) :
    IVertexBuffer(buffer_cnt, attr_cnt, element_cnt, attributes),
    buffers_(buffer_cnt, nullptr) {

}
}