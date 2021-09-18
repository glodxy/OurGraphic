//
// Created by Glodxy on 2021/8/31.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_ITEXTURE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_ITEXTURE_H_
#include "Framework/Backend/include_internal/IResource.h"
#include "IRenderDevice.h"
#include "../include_internal/ITextureView.h"
namespace our_graph {
 class ITexture : public SpecResource<IBuffer, ITextureView> {
 public:
  virtual void Create() = 0;
  virtual void Destroy() = 0;
  virtual ~ITexture() override {};
  virtual std::shared_ptr<IBuffer> GetBuffer() = 0;
  virtual std::shared_ptr<ITextureView> GetView() = 0;
};

} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_ITEXTURE_H_
