//
// Created by Glodxy on 2021/8/31.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IDESCRIPTOR_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IDESCRIPTOR_H_
#include "IBuffer.h"
namespace our_graph {
/**
 * Descripto作为资源的描述符，通常与Buffer一同构造为一个Resource。、
 * 用于描述如何去解释Buffer
 * */
class IDescriptor {
 public:
  virtual void Create(std::shared_ptr<IBuffer> buffer) = 0;
  virtual void BindBuffer(std::shared_ptr<IBuffer> buffer) = 0;
  virtual void Destroy() = 0;
  virtual void* GetInstance() = 0;
 protected:
  std::shared_ptr<IBuffer> buffer_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IDESCRIPTOR_H_
