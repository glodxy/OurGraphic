//
// Created by Glodxy on 2022/1/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_PASS_RENDERGRAPHRENDERPASS_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_PASS_RENDERGRAPHRENDERPASS_H_
#include "Backend/include/DriverEnum.h"
#include "Renderer/RenderGraph/Resource/RenderGraphTexture.h"

#include "Renderer/RenderGraph/Base/RenderGraphId.h"
#include "Utils/Math/Math.h"

namespace our_graph::render_graph {

/**
 * 该类仅用来标志了一个通用的render pass所需要的资源
 * */
struct RenderGraphRenderPassInfo {
  // 除了render target，还要带上depth、stencil
  static constexpr size_t ATTACHMENT_COUNT = MAX_SUPPORTED_RENDER_TARGET_COUNT + 2;

  struct Attachments {
    union {
      RenderGraphId<RenderGraphTexture> array[ATTACHMENT_COUNT] = {};
      struct {
        RenderGraphId<RenderGraphTexture> color[MAX_SUPPORTED_RENDER_TARGET_COUNT];
        RenderGraphId<RenderGraphTexture> depth;
        RenderGraphId<RenderGraphTexture> stencil;
      };
    };
  };

  struct Descriptor {
    // render target
    Attachments attachments {};
    Viewport view_port{};
    // 清除的颜色
    math::Vec4 clear_color{};
    uint8_t samples = 1;
    double clear_depth = 1.0f;
    // 要清除的attachment
    TargetBufferFlags clear_flags{TargetBufferFlags::NONE};
    // 要丢弃的attachment
    TargetBufferFlags discard_start {TargetBufferFlags::NONE};
  };

  struct ExternalDescriptor {
    // 要绑定的位置(默认color0)
    TargetBufferFlags attachments = TargetBufferFlags::COLOR0;
    Viewport view_port{};
    math::Vec4 clear_color{};
    uint8_t samples = 0;
    TargetBufferFlags clear_flags;
    TargetBufferFlags keep_override_start{};
    TargetBufferFlags keep_override_end{};
  };

  uint32_t id = 0;
};

}  // namespace our_graph::render_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_PASS_RENDERGRAPHRENDERPASS_H_
