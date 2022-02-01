//
// Created by glodxy on 2022/1/26.
//

#include "DeferredRenderer.h"

#include "Resource/include/GlobalShaders.h"
#include "include/GlobalEnum.h"

#include "Framework/Component/PerViewUniform.h"
#include "Resource/include/MaterialInstance.h"
#include "Resource/include/Material.h"
#include "Resource/include/MeshReader.h"
namespace our_graph {
using namespace our_graph::render_graph;

DeferredRenderer::DeferredRenderer(Driver *driver) : SceneRenderer(driver) {
  default_rt_ = driver->CreateDefaultRenderTarget();
}


void DeferredRenderer::PrepareGeometryPass(RenderGraph& graph) {
  graph.AddTrivialSideEffectPass("PrepareRenderable", [&](Driver* driver) {
    //todo 1. 设置所有renderable的uniform
    mesh_collector_.CommitPerRenderableUniforms();
  });
  graph.AddTrivialSideEffectPass("PrepareMaterial", [&](Driver* driver) {
    // todo:提交所有material
    scene_.CommitAllMaterialInstance(driver);
  });

  struct BasePassParams {
    PerViewUniform* per_view;
    PipelineState pipeline_state;
  };
  // 对于每个view的渲染处理
  auto render_per_view_base_pass = [&](ViewInfo& view, uint32_t view_idx) {
    // 1. setup perview的uniform
    graph.AddPass<BasePassParams>("BasePass",
                                  [&](RenderGraph::Builder& builder, BasePassParams& params) {
      // 创建gbuffer
      // 用于创建gbuffer的desc
      RenderGraphTexture::Descriptor gbuffer_desc;
      gbuffer_desc.type = SamplerType::SAMPLER_2D;
      gbuffer_desc.format = TextureFormat::RGBA8;
      gbuffer_desc.width = width_;
      gbuffer_desc.height = height_;
      gbuffer_desc.samples = 1;
      // 用于创建rt的desc todo
      RenderGraphRenderPassInfo::Descriptor rt_desc;
      rt_desc.samples = 1;
      rt_desc.view_port.left = 0;
      rt_desc.view_port.bottom= 0;
      for (int i = 0; i < GBUFFER_MAX_SIZE; ++i) {
        gbuffer_data_.textures[i] = builder.CreateTexture(std::string("gBuffer") + char('A' + i),
                              gbuffer_desc);
        gbuffer_data_.textures[i] = builder.Write(gbuffer_data_.textures[i]);
        rt_desc.attachments.color[i] = gbuffer_data_.textures[i];
      }
      builder.DeclareRenderPass("gbuffer", rt_desc);
      // todo:此处完成params的初始化
      params.per_view = view.GetUniforms();
    },
                                  [&](const RenderGraphResources& resources, const BasePassParams& params, Driver* driver) {
      //todo:此处根据params来完成渲染
      /**
       * todo:
       * 1. 绑定per view
       * for :
       *    2. 绑定per renderable
       *    3. use对应的material
       *    4. 获取primitive以及shader
       *
       */
      // 提交per view uniform
      params.per_view->Commit();
      // 1. 绑定per view
      params.per_view->Bind();

      auto pass_info = resources.GetRenderPassInfo();
      driver->BeginRenderPass(pass_info.target, std::move(pass_info.params));
      PipelineState tmp_state = params.pipeline_state;
      // 遍历得到所有的renderable
      for (int i = 0; i < mesh_collector_.GetSize(); ++i) {
        //  使用per renderable
        mesh_collector_.UsePerRenderableUniform(i);
        // 使用material
        MaterialInstance* mat = scene_.GetMaterialInstance(i);
        mat->Use();
        ShaderHandle shader = mat->GetMaterial()->GetShader(0);
        RenderPrimitiveHandle primitive = mesh_collector_.GetRenderPrimitiveAt(i);
        tmp_state.shader_ = shader;
        driver->Draw(tmp_state, primitive);
      }
      driver->EndRenderPass();
    });

  };

  // 对每个view添加pass
  for (int view_idx = 0; view_idx < views_.size(); ++view_idx) {
    auto& view = views_[view_idx];
    render_per_view_base_pass(view, view_idx);
  }
}

void DeferredRenderer::PrepareLightPass(render_graph::RenderGraph &graph) {
  struct LightPassParam {
    PerViewUniform* per_view;
    PipelineState pipeline_state;
    SamplerGroupHandle sampler_group_handle;
  };
  RenderGraphRenderPassInfo::ExternalDescriptor desc;
  RenderGraphId<RenderGraphTexture> default_rt_id = graph.Import("swap_chain", desc, default_rt_);
  // 对于每个view的渲染处理
  auto render_per_view_base_pass = [&](ViewInfo& view, uint32_t view_idx) {
    // 1. setup perview的uniform
    graph.AddPass<LightPassParam>("LightPass",
                                  [&](RenderGraph::Builder& builder, LightPassParam& params) {
        builder.DeclareRenderPass(default_rt_id);
        for (int i = 0; i < GBUFFER_MAX_SIZE; ++i) {
          gbuffer_data_.textures[i] = builder.Sample(gbuffer_data_.textures[i]);
        }
        params.per_view = view.GetUniforms();

        params.sampler_group_handle = driver_->CreateSamplerGroup(GBUFFER_MAX_SIZE);
        params.pipeline_state.shader_ = GlobalShaders::Get().GetGlobalShader(GlobalShaderType::DEFERRED_LIGHT);
        // todo:此处完成params的初始化
        params.pipeline_state.raster_state_.colorWrite = true;
        params.pipeline_state.raster_state_.culling = CullingMode::NONE;
        params.pipeline_state.raster_state_.depthFunc = SamplerCompareFunc::A;
      },
      [&](const RenderGraphResources& resources, const LightPassParam& params, Driver* driver) {
      //todo:此处根据params来完成渲染
      /**
       * todo:
       * 1. 绑定per view
       * 2. 绑定sampler
       **/
      params.per_view->Commit();
      // 1. 绑定per view
      params.per_view->Bind();

      driver_->BindSamplers(0, params.sampler_group_handle);

        // 2. 绑定sampler
      SamplerGroup sampler_group(5);
      SamplerParams gbuffer_sampler_param;
      gbuffer_sampler_param.u = 0;
      for (int i = 0; i < GBUFFER_MAX_SIZE; ++i) {
        auto handle = resources.GetTexture(gbuffer_data_.textures[i]);
        sampler_group.SetSampler(i, handle, gbuffer_sampler_param);
      }
      driver->UpdateSamplerGroup(params.sampler_group_handle, std::move(sampler_group));

      auto pass_info = resources.GetRenderPassInfo();
      driver->BeginRenderPass(pass_info.target, std::move(pass_info.params));
      // todo:获取texture quad的primitive
      RenderPrimitiveHandle primitive = MeshReader::GetQuadPrimitive();
      driver->Draw(params.pipeline_state, primitive);
      driver->EndRenderPass();

      driver->DestroySamplerGroup(params.sampler_group_handle);
    });
  };

  // 对每个view添加pass
  for (int view_idx = 0; view_idx < views_.size(); ++view_idx) {
    auto& view = views_[view_idx];
    render_per_view_base_pass(view, view_idx);
  }
}

void DeferredRenderer::Init() {

}

void DeferredRenderer::Render() {
  if (!render_graph_) {
    render_graph_ = new RenderGraph(allocator_);
  }
  auto& graph = render_graph_;
  PrepareGeometryPass(*render_graph_);
  PrepareLightPass(*render_graph_);
  render_graph_->Compile();

  graph->Execute(driver_);

  delete render_graph_;
  render_graph_ = nullptr;
}

void DeferredRenderer::Destroy() {
  SceneRenderer::Destroy();
  driver_->DestroyRenderTarget(default_rt_);
}

}