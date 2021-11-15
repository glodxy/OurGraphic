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
  // todo:数据解释方式，目前默认byte
  ElementType elem_type {ElementType::BYTE};
  SoftBuffer() = default;
  SoftBuffer(uint32_t bytes);
  SoftBuffer(ElementType type, uint32_t cnt);
  ~SoftBuffer();
};

class SoftVertexBuffer : public IVertexBuffer {
 public:
  explicit SoftVertexBuffer(uint8_t buffer_cnt,
                            uint8_t attr_cnt, uint32_t element_cnt,
                            const AttributeArray& attributes);

  std::vector<SoftBuffer*> buffers_;
};

class SoftIndexBuffer : public IIndexBuffer {
 public:
  /**
   * @param element_type:每个index元素的类型
   * @param index_cnt:index的数目
   * */
  explicit SoftIndexBuffer(ElementType element_type, uint32_t index_cnt);
  ~SoftIndexBuffer();

  SoftBuffer* buffer_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTBUFFER_H_
