//
// Created by Glodxy on 2021/8/31.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IRESOURCE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IRESOURCE_H_
#include "IBuffer.h"
#include "IDescriptor.h"
#include <memory>
namespace our_graph {
template<class T>
using CheckBufferType = std::enable_if<std::is_base_of<IBuffer, T>::value>;

template<class T>
using CheckDescriptor = std::enable_if<std::is_base_of<IDescriptor, T>::value>;

template<class BufferType = IBuffer, class DescriptorType = IDescriptor,
    typename = CheckBufferType<BufferType>,
    typename = CheckDescriptor<DescriptorType>,
    typename ...Args>
class IResource {
 public:
  virtual void Create(Args... args) = 0;
  virtual void Destroy() = 0;
 protected:
  std::shared_ptr<BufferType> buffer_{nullptr};
  std::shared_ptr<DescriptorType> descriptor_{nullptr};
};

template<typename ...Args>
using BaseResource = IResource<IBuffer, IDescriptor, void, void, Args...>;

template<class Buffer, class Descriptor, typename  ...Args>
using SpecResource = IResource<Buffer, Descriptor,
                               CheckBufferType<Buffer>, CheckDescriptor<Descriptor>, Args...>;
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IRESOURCE_H_
