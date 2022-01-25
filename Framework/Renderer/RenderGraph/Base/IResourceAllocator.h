//
// Created by Glodxy on 2022/1/23.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_IRESOURCEALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_IRESOURCEALLOCATOR_H_
#include "Backend/include/DriverEnum.h"
#include "Backend/include/Handle.h"
#include "Backend/include/RenderTargetInfo.h"

namespace our_graph::render_graph {

class ResourceAllocatorInterface {
 public:
  virtual RenderTargetHandle CreateRenderTarget(const std::string& name,
                                                TargetBufferFlags targetBufferFlags,
                                                uint32_t width,
                                                uint32_t height,
                                                uint8_t samples,
                                                MRT color,
                                                TargetBufferInfo depth,
                                                TargetBufferInfo stencil) noexcept {}

  virtual void DestroyRenderTarget(RenderTargetHandle h) noexcept {}

  virtual TextureHandle CreateTexture(const std::string& name, SamplerType target,
                                      uint8_t levels, TextureFormat format, uint8_t samples,
                                      uint32_t width, uint32_t height, uint32_t depth,
                                      std::array<TextureSwizzle, 4> swizzle,
                                      TextureUsage usage) noexcept {}

  virtual void DestroyTexture(TextureHandle h) noexcept {}
};

}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_IRESOURCEALLOCATOR_H_
