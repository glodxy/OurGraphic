//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_ICOMMANDBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_ICOMMANDBUFFER_H_

namespace our_graph {
class ICommandBuffer {
 public:
  virtual void Create() = 0;
  virtual void Destroy() = 0;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_ICOMMANDBUFFER_H_
