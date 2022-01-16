//
// Created by Glodxy on 2022/1/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RESOURCE_RENDERGRAPHRESOURCE_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RESOURCE_RENDERGRAPHRESOURCE_H_
#include "Backend/include/DriverEnum.h"
#include "Backend/include/Handle.h"

#include "Renderer/RenderGraph/Base/RenderGraphId.h"
#include "Renderer/RenderGraph/Resource/RenderGraphTexture.h"
#include "Renderer/RenderGraph/Base/Resource.h"
namespace our_graph::render_graph {

class RenderGraph;
class PassNode;
class VirtualResource;


/**
 * 该类标识了一个pass所需要的资源
 * */
class RenderGraphResources {
 public:
  RenderGraphResources(RenderGraph& rg, PassNode& pass_node) noexcept;
  RenderGraphResources(const RenderGraphResources&) = delete;
  RenderGraphResources& operator=(const RenderGraphResources&) = delete;

  struct RenderPassInfo {
    RenderTargetHandle target;
    RenderPassParams params;
  };

  // 获取执行的pass的name
  std::string GetPassName() const noexcept;

  // 根据id获取对应的资源
  template<class RESOURCE>
  const RESOURCE& Get(RenderGraphId<RESOURCE> handle) const;

  // 获取对应的Desc
  template<class RESOURCE>
  const typename RESOURCE::Descriptor& GetDescriptor(RenderGraphId<RESOURCE> handle) const;

  template<class RESOURCE>
  const typename RESOURCE::SubResourceDescriptor& GetSubResourceDescriptor(RenderGraphId<RESOURCE> handle) const;

  template<class RESOURCE>
  const typename RESOURCE::Usage& GetUsage(RenderGraphId<RESOURCE> handle) const;


  /**
   * 标识一个资源，让其不再受render graph的管理
   * */
  template<class RESOURCE>
  void Detach(RenderGraphId<RESOURCE> handle,
              RESOURCE* out_resource, typename RESOURCE::Descriptor* out_desc) const;

  // todo
  RenderPassInfo GetRenderPassInfo(uint32_t id = 0u) const;

  TextureHandle  GetTexture(RenderGraphId<RenderGraphTexture> handle) const noexcept {
    return Get(handle).handle_;
  }

 private:
  VirtualResource& GetResource(RenderGraphHandle handle) const;

  RenderGraph& render_graph_;
  PassNode& pass_node_;
};

template<class RESOURCE>
const RESOURCE& RenderGraphResources::Get(RenderGraphId<RESOURCE> handle) const {
  return static_cast<const Resource<RESOURCE>&>(GetResource(handle)).resource_;
}

template<class RESOURCE>
const typename RESOURCE::Descriptor &RenderGraphResources::GetDescriptor(RenderGraphId<RESOURCE> handle) const {
  return static_cast<const Resource<RESOURCE>&>(GetResource(handle)).descriptor_;
}

template<class RESOURCE>
const typename RESOURCE::SubResourceDescriptor &RenderGraphResources::GetSubResourceDescriptor(RenderGraphId<RESOURCE> handle) const {
  return static_cast<const Resource<RESOURCE>&>(GetResource(handle)).sub_resource_descriptor_;
}

template<class RESOURCE>
const typename RESOURCE::Usage &RenderGraphResources::GetUsage(RenderGraphId<RESOURCE> handle) const {
  return static_cast<const Resource<RESOURCE>&>(GetResource(handle)).usage_;
}

template<class RESOURCE>
void RenderGraphResources::Detach(RenderGraphId<RESOURCE> handle,
                                  RESOURCE *out_resource,
                                  typename RESOURCE::Descriptor *out_desc) const {
  Resource<RESOURCE>& concrete = static_cast<Resource<RESOURCE>&>(GetResource(handle));
  concrete.detached_ = true;
  *out_resource = concrete.resource_;
  if (out_desc) {
    *out_desc = concrete.descriptor_;
  }
}

}  // namespace our_graph::render_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_RESOURCE_RENDERGRAPHRESOURCE_H_
