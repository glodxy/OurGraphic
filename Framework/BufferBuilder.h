//
// Created by Glodxy on 2021/10/14.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_
#define OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_

#include "Backend/include/Driver.h"
#include "include/VertexBuffer.h"
#include "include/IndexBuffer.h"

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
 public:
  static VertexBuffer* BuildDefaultVertex(DriverApi* driver);

  static IndexBuffer* BuildDefaultIndex(DriverApi* driver);
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_
