//
// Created by Glodxy on 2021/9/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ITEXTUREVIEW_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ITEXTUREVIEW_H_
#include "IDescriptor.h"

namespace our_graph {
class ITextureView : public IDescriptor {
 public:
  virtual void Create(std::shared_ptr<IBuffer> buffer) = 0;
  virtual void BindBuffer(std::shared_ptr<IBuffer> buffer) = 0;
  virtual void Destroy() = 0;

  virtual void* GetInstance() {
    return nullptr;
  }
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ITEXTUREVIEW_H_
