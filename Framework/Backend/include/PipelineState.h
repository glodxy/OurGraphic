//
// Created by Glodxy on 2021/10/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_PIPELINESTATE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_PIPELINESTATE_H_
#include <climits>
#include "Handle.h"
#include "DriverEnum.h"
namespace our_graph {

class PipelineState {
 public:
  ShaderHandle shader_;
  RasterState raster_state_;
  PolygonOffset polygon_offset_;
  Viewport scissor_ {0, 0,
                    (uint32_t)std::numeric_limits<int32_t>::max(),
                    (uint32_t)std::numeric_limits<int32_t>::min()};
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_PIPELINESTATE_H_
