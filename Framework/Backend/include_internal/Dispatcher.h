//
// Created by Glodxy on 2021/10/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DISPATCHER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DISPATCHER_H_

#include "Commands.h"
namespace our_graph {

template<class DriverClass>
class Dispatcher : public DispatcherBase {
#define MAP_API(methodName) methodName##_ = &Dispatcher::methodName;
 public:
  explicit Dispatcher() noexcept : DispatcherBase() {
    MAP_API(Tick);
    MAP_API(BeginFrame);
    MAP_API(EndFrame);
    MAP_API(Flush);
    MAP_API(Finish);
    MAP_API(BeginRenderPass);
    MAP_API(EndRenderPass);
    MAP_API(SetRenderPrimitiveBuffer);
    MAP_API(SetRenderPrimitiveRange);
    MAP_API(MakeCurrent);
    MAP_API(Commit);
    MAP_API(Draw);
    MAP_API(Blit);

    MAP_API(CreateVertexBuffer);
    MAP_API(CreateIndexBuffer);
    MAP_API(CreateBufferObject);
    MAP_API(CreateRenderPrimitive);
    MAP_API(CreateShader);
    MAP_API(CreateDefaultRenderTarget);
    MAP_API(CreateRenderTarget);
    MAP_API(CreateSwapChain);
    MAP_API(CreateTexture);
    MAP_API(CreateSamplerGroup);

    MAP_API(DestroyVertexBuffer);
    MAP_API(DestroyIndexBuffer);
    MAP_API(DestroyBufferObject);
    MAP_API(DestroyRenderPrimitive);
    MAP_API(DestroyShader);
    MAP_API(DestroyRenderTarget);
    MAP_API(DestroySwapChain);
    MAP_API(DestroyTexture);
    MAP_API(DestroySamplerGroup);

    MAP_API(SetVertexBufferObject);
    MAP_API(UpdateIndexBuffer);
    MAP_API(UpdateBufferObject);
    MAP_API(Update2DImage);
    MAP_API(UpdateCubeImage);
    MAP_API(GenerateMipmaps);
    MAP_API(UpdateSamplerGroup);

    MAP_API(BindUniformBuffer);
    MAP_API(BindUniformBufferRange);
    MAP_API(BindSamplers);

    MAP_API(ReadPixels);
  }
 private:
#define DECL_API(methodName) \
  static void methodName(DriverApi& driver, CommandBase* base, intptr_t* next) { \
    using Cmd = COMMAND_TYPE(methodName);                                        \
    DriverClass& real_driver = static_cast<DriverClass&>(driver);                \
    Cmd::Execute(&DriverClass::methodName, real_driver, base, next);\
  }

#define DECL_API_RETURN(methodName) \
  static void methodName(DriverApi& driver, CommandBase* base, intptr_t* next) { \
    using Cmd = COMMAND_TYPE(methodName##R);                                     \
    DriverClass& real_driver = static_cast<DriverClass&>(driver);                \
    Cmd::Execute(&DriverClass::methodName##R, real_driver, base, next);\
  }

  DECL_API(Tick);
  DECL_API(BeginFrame);
  DECL_API(EndFrame);
  DECL_API(Flush);
  DECL_API(Finish);
  DECL_API(BeginRenderPass);
  DECL_API(EndRenderPass);
  DECL_API(SetRenderPrimitiveBuffer);
  DECL_API(SetRenderPrimitiveRange);
  DECL_API(MakeCurrent);
  DECL_API(Commit);
  DECL_API(Draw);
  DECL_API(Blit);

  DECL_API_RETURN(CreateVertexBuffer);
  DECL_API_RETURN(CreateIndexBuffer);
  DECL_API_RETURN(CreateBufferObject);
  DECL_API_RETURN(CreateRenderPrimitive);
  DECL_API_RETURN(CreateShader);
  DECL_API_RETURN(CreateDefaultRenderTarget);
  DECL_API_RETURN(CreateRenderTarget);
  DECL_API_RETURN(CreateSwapChain);
  DECL_API_RETURN(CreateTexture);
  DECL_API_RETURN(CreateSamplerGroup);

  DECL_API(DestroyVertexBuffer);
  DECL_API(DestroyIndexBuffer);
  DECL_API(DestroyBufferObject);
  DECL_API(DestroyRenderPrimitive);
  DECL_API(DestroyShader);
  DECL_API(DestroyRenderTarget);
  DECL_API(DestroySwapChain);
  DECL_API(DestroyTexture);
  DECL_API(DestroySamplerGroup);

  DECL_API(SetVertexBufferObject);
  DECL_API(UpdateIndexBuffer);
  DECL_API(UpdateBufferObject);
  DECL_API(Update2DImage);
  DECL_API(UpdateCubeImage);
  DECL_API(GenerateMipmaps);
  DECL_API(UpdateSamplerGroup);

  DECL_API(BindUniformBuffer);
  DECL_API(BindUniformBufferRange);
  DECL_API(BindSamplers);

  DECL_API(ReadPixels);
};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DISPATCHER_H_
