//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPLATFORMSDL_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPLATFORMSDL_H_

#include "Backend/include_internal/IPlatform.h"

namespace our_graph {
class SoftPlatformSDL : public IPlatform {
 public:
  bool CreateSurface(void *native_window, void *instance, uint64_t flags, void *surface) override;
  std::vector<const char *> GetInstanceExtLayers() const override{
    return std::vector<const char*>();
  }
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTPLATFORMSDL_H_
