//
// Created by Glodxy on 2022/1/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
#include <cstdint>
#include <string>

#include "Renderer/RenderGraph/Pass/RenderGraphRenderPass.h"
#include "Renderer/RenderGraph/Base/RenderGraphId.h"
namespace our_graph::render_graph {

class ResourceAllocatorInterface;

class RenderGraphPassExecutor;
class PassNode;
class ResourceNode;
class VirtualResource;

// todo
class RenderGraph {
 public:
  class Builder {
   public:
    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;

    /**
     * 生成一个在当前pass中使用的render pass
     * @param name:pass的名称
     * @param desc:所需要的descriptor
     * @return 该render pass的idx
     * */
    uint32_t DeclareRenderPass(const std::string& name,
                               const RenderGraphRenderPassInfo::Descriptor& desc);


    RenderGraphId<RenderGraphTexture> DeclareRenderPass(
        RenderGraphId<RenderGraphTexture> color, uint32_t* idx = nullptr);
  };
};
}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RENDERGRAPH_H_
