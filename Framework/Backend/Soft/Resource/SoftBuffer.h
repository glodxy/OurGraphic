//
// Created by Glodxy on 2021/11/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTBUFFER_H_
#include <vector>

#include "Backend/include_internal/IResource.h"
namespace our_graph {
struct SoftBuffer : public IBufferObject {
  void* buffer {nullptr};  // 缓冲区
  // todo:数据解释方式，目前默认float3
  ElementType elem_type {ElementType::FLOAT3};
};

class SoftVertexBuffer : public IVertexBuffer {
 public:
  explicit SoftVertexBuffer(uint8_t buffer_cnt,
                            uint8_t attr_cnt, uint32_t element_cnt,
                            const AttributeArray& attributes);

  std::vector<SoftBuffer*> buffers_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTBUFFER_H_
