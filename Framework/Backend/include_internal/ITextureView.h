//
// Created by Glodxy on 2021/9/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ITEXTUREVIEW_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ITEXTUREVIEW_H_
#include "IDescriptor.h"

namespace our_graph {
class ITextureView : public IDescriptor {
 public:

  virtual void* GetInstance() override {
    return nullptr;
  }

  virtual void * GetDescription() override {
    return nullptr;
  }
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ITEXTUREVIEW_H_
