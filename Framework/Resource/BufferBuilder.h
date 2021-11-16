//
// Created by Glodxy on 2021/10/14.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_
#define OUR_GRAPHIC_FRAMEWORK_BUFFERBUILDER_H_

#include "Framework/Resource/include/VertexBuffer.h"
#include "Framework/Resource/include/IndexBuffer.h"
#include "Framework/Resource/include/BufferObject.h"

namespace our_graph {
class BufferBuilder {
  static constexpr float default_vertex[4][4] = {
    {1.0f, 1.0f, .0f, 1.f},
    {-1.0f, 1.0f, .0f, 1.f},
    {1.0f, -1.0f, .0f, 1.f},
    {-1.0f, -1.0f, .0f, 1.f},
  };

  static constexpr uint32_t default_index[12][3] = {
      {0, 1, 2},
      {3, 4, 5},
      {6, 7, 8},
      {9, 10, 11},
      {12, 13, 14},
      {15, 16, 17},
      {18, 19, 20},
      {21, 22, 23},
      {24, 25, 26},
      {27, 28, 29},
      {30, 31, 32},
      {33, 34, 35}
  };

  static constexpr float default_soft_vertex[36][3] = {
      // front
      {0, 0, 0},
      {1 ,1, 0},
      {0, 1, 0},
      {1, 0, 0},
      {1, 1, 0},
      {0 ,0 ,0},

      // back
      {0, 1, -1},
      {1, 1, -1},
      {0 ,0, -1},
      {1, 1, -1},
      {1, 0, -1},
      {0, 0, -1},

      // left
      {0, 0, -1},
      {0, 1, 0},
      {0, 1, -1},
      {0, 0, -1},
      {0, 0, 0},
      {0, 1, 0},

      // right
      {1, 0, 0},
      {1, 1, -1},
      {1, 1, 0},
      {1, 0, 0},
      {1, 0, -1},
      {1, 1, -1},

      //top
      {0, 1, 0},
      {1, 1, 0},
      {1, 1, -1},
      {0, 1, 0},
      {1, 1, -1},
      {0, 1, -1},

      // bot
      {0, 0, 0},
      {0, 0, -1},
      {1, 0, 0},
      {0, 0, -1},
      {1, 0, -1},
      {1, 0, 0}
  };

  static constexpr float default_soft_normal[36][3] = {
      // front
      {0, 0, 1},
      {0, 0, 1},
      {0, 0, 1},
      {0, 0, 1},
      {0, 0, 1},
      {0, 0, 1},

      // back
      {0, 0, -1},
      {0, 0, -1},
      {0, 0, -1},
      {0, 0, -1},
      {0, 0, -1},
      {0, 0, -1},

      // left
      {-1, 0, 0},
      {-1, 0, 0},
      {-1, 0, 0},
      {-1, 0, 0},
      {-1, 0, 0},
      {-1, 0, 0},

      // right
      {1, 0, 0},
      {1, 0, 0},
      {1, 0, 0},
      {1, 0, 0},
      {1, 0, 0},
      {1, 0, 0},

      // top
      {0, 1, 0},
      {0, 1, 0},
      {0, 1, 0},
      {0, 1, 0},
      {0, 1, 0},
      {0, 1, 0},

      // bot
      {0, -1, 0},
      {0, -1, 0},
      {0, -1, 0},
      {0, -1, 0},
      {0, -1, 0},
      {0, -1, 0},
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
