//
// Created by Glodxy on 2021/11/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTRENDERPRIMITIVE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTRENDERPRIMITIVE_H_
#include "Backend/include_internal/IResource.h"
#include "Backend/include/DriverEnum.h"
namespace our_graph {
class SoftVertexBuffer;
/**
 * 该类用于标识渲染原型，包括顶点缓冲、索引缓冲、拓扑形状
 * 目前仅有顶点缓冲
 * todo:index buffer, topology
 * */
class SoftRenderPrimitive : public IRenderPrimitive {
 public:
  explicit SoftRenderPrimitive();

  void BindVertex(SoftVertexBuffer* vertex_buffer);

  void* const GetVertexData() const;
  size_t GetVertexCnt() const;
 private:
  // 顶点缓冲区
  SoftVertexBuffer* buffer_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTRENDERPRIMITIVE_H_
