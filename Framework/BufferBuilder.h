//
// Created by Glodxy on 2021/10/14.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_
#define OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_

#include "include/VertexBuffer.h"
#include "include/IndexBuffer.h"
#include "include/BufferObject.h"

namespace our_graph {
class BufferBuilder {
  static constexpr float default_vertex[4][4] = {
    {1.0f, 1.0f, .0f, 1.f},
    {-1.0f, 1.0f, .0f, 1.f},
    {1.0f, -1.0f, .0f, 1.f},
    {-1.0f, -1.0f, .0f, 1.f},
  };

  static constexpr uint32_t default_index[2][3] = {
      {1, 0, 3},
      {3, 0, 2}
  };

  static float window_rect[2];
 public:
  static VertexBuffer* BuildDefaultVertex(Driver* driver);

  static IndexBuffer* BuildDefaultIndex(Driver* driver);

  static BufferObject* BuildDefaultQuadUniformBuffer(Driver* driver,
                                                     float width,
                                                     float height);
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_
