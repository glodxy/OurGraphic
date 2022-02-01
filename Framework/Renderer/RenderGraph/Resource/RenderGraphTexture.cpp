//
// Created by Glodxy on 2022/1/15.
//

#include "RenderGraphTexture.h"
#include "Renderer/RenderGraph/Base/IResourceAllocator.h"
#include "Renderer/RenderGraph/Base/RenderGraphUtils.h"
#include <map>
namespace our_graph::render_graph {
static const std::map<TextureUsage, std::string> kUsageStrMap = {
    {TextureUsage::NONE, "None"},
    {TextureUsage::SAMPLEABLE, "Samplerable"},
    {TextureUsage::COLOR_ATTACHMENT, "Color"},
    {TextureUsage::DEFAULT, "Default"},
    {TextureUsage::DEPTH_ATTACHMENT, "Depth"},
    {TextureUsage::STENCIL_ATTACHMENT, "Stencil"},
    {TextureUsage::SUBPASS_INPUT, "Subpass"},
    {TextureUsage::UPLOADABLE, "Upload"}
};

template<>
std::string ToString<RenderGraphTexture::Usage>(RenderGraphTexture::Usage usage) {

  return kUsageStrMap.at(usage);
}
void RenderGraphTexture::Create(ResourceAllocatorInterface &allocator,
                                const std::string &name,
                                const Descriptor &desc,
                                Usage usage) noexcept {
  std::array<TextureSwizzle, 4> swizzle = {
      desc.swizzle.r,
      desc.swizzle.g,
      desc.swizzle.b,
      desc.swizzle.a };
  handle = allocator.CreateTexture(name,
                                   desc.type, desc.levels, desc.format, desc.samples,
                                   desc.width, desc.height, desc.depth,
                                   swizzle, usage);
}

void RenderGraphTexture::Destroy(ResourceAllocatorInterface &allocator) noexcept {
  if (handle) {
    allocator.DestroyTexture(handle);
    handle.Clear();
  }
}

RenderGraphTexture::Descriptor RenderGraphTexture::GenerateSubResourceDescriptor(Descriptor desc,
                                                             const SubResourceDescriptor &sub) noexcept {
  desc.levels = 1;
  desc.width = std::max(1u, desc.width >> sub.level);
  desc.height = std::max(1u, desc.height >> sub.level);
  return desc;
}

}  // namespace our_graph::render_graph