//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IRENDERINSTANCE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IRENDERINSTANCE_H_
#include <memory>
namespace our_graph {
class IRenderInstance {
 public:
  virtual void CreateInstance() = 0;

  virtual void DestroyInstance() = 0;

 protected:

};
} // namespace our_graph


#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IRENDERINSTANCE_H_
