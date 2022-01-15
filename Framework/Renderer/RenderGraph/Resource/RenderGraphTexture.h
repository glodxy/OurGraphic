//
// Created by Glodxy on 2022/1/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RESOURCE_RENDERGRAPHTEXTURE_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RESOURCE_RENDERGRAPHTEXTURE_H_
#include "Backend/include/Handle.h"
#include "Backend/include/DriverEnum.h"
namespace our_graph::render_graph {

class ResourceAllocatorInterface;

struct RenderGraphTexture {
  TextureHandle handle;

  struct Descriptor {
    uint32_t width = 1; // 资源的width
    uint32_t height = 1; // 资源的height
    uint32_t depth = 1; // 3d纹理使用
    uint8_t levels = 1; // 资源的levels
    uint8_t samples = 0; // 采样数
    SamplerType type = SamplerType::SAMPLER_2D;
    TextureFormat format = TextureFormat::RGBA8;

    struct {
      union {
        TextureSwizzle channels[4] = {
            TextureSwizzle::CHANNEL_0,
            TextureSwizzle::CHANNEL_1,
            TextureSwizzle::CHANNEL_2,
            TextureSwizzle::CHANNEL_3
        };
        struct {
          TextureSwizzle r,g,b,a;
        };
      };
    } swizzle;
  };

  struct SubResourceDescriptor {
    uint8_t level = 0;  // resource的mipmap
    uint8_t layer = 0;  // layer/face
  };

  using Usage = TextureUsage;
  static constexpr Usage DEFAULT_R_USAGE = Usage::SAMPLEABLE;
  static constexpr Usage DEFAULT_W_USAGE = Usage::COLOR_ATTACHMENT;

  void Create(ResourceAllocatorInterface& allocator,
              const std::string& name, const Descriptor& desc, Usage usage) noexcept;

  void Destroy(ResourceAllocatorInterface& allocator) noexcept;

  /**
   * 通过子资源以及parent的desc生成应有的descriptor
   * @param desc:parenmt的描述符
   * @param sub:子资源的描述符
   * */
  static Descriptor GenerateSubResourceDescriptor(Descriptor desc,
                                                  const SubResourceDescriptor& sub) noexcept;
};
}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RESOURCE_RENDERGRAPHTEXTURE_H_
