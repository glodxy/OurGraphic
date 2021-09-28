//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_IPLATFORM_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_IPLATFORM_H_
#include <vector>

namespace our_graph {
class IPlatform {
 public:
  // 获取instance扩展的layer
  virtual std::vector<const char*> GetInstanceExtLayers()const = 0;

  virtual bool CreateSurface(void* native_window, void* instance, uint64_t flags, void* surface) = 0;

};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_IPLATFORM_H_
