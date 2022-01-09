//
// Created by Glodxy on 2021/10/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCEALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCEALLOCATOR_H_
#include <memory>
#include <set>
#include <unordered_map>
#include "Framework/Resource/include_internal/ResourceBase.h"
#include "Framework/Resource/include/VertexBuffer.h"
#include "Framework/Resource/include/IndexBuffer.h"
#include "Framework/Resource/include/BufferObject.h"
#include "Framework/Resource/include/Texture.h"
#include "Framework/Resource/include/Material.h"
#include "Framework/Resource/include/MaterialInstance.h"
#include "Framework/Resource/include/RenderTarget.h"

namespace our_graph {
class ResourceAllocator {
 public:
  template<class T,
      typename = std::enable_if<std::is_base_of_v<ResourceBase, T>>>
  T* CreateResource(const typename T::Builder& builder) {
    T* ptr = Construct<T>(builder);
    common_resources_.insert(ptr);
    return ptr;
  }

  VertexBuffer* CreateVertexBuffer(const VertexBuffer::Builder &builder) {
    VertexBuffer* ptr = Construct<VertexBuffer>(builder);
    vertex_buffers_.insert(ptr);
    return ptr;
  }

  IndexBuffer* CreateIndexBuffer(const IndexBuffer::Builder& builder) {
    IndexBuffer* p = Construct<IndexBuffer>(builder);
    index_buffers_.insert(p);
    return p;
  }

  BufferObject* CreateBufferObject(const BufferObject::Builder& builder) {
    BufferObject* p = Construct<BufferObject>(builder);
    buffer_objects_.insert(p);
    return p;
  }

  Texture* CreateTexture(const Texture::Builder& builder) {
    Texture* tex = Construct<Texture>(builder);
    textures_.insert(tex);
    return tex;
  }

  Material* CreateMaterial(const Material::Builder& builder) {
    Material* mat = Construct<Material>(builder);
    materials_.insert(mat);
    return mat;
  }

  MaterialInstance* CreateMaterialInstance(const MaterialInstance* other, const std::string& name) {
    MaterialInstance* material_instance = Construct<MaterialInstance>(other, name);
    const Material* src_mat = material_instance->GetMaterial();
    material_instances_[src_mat].insert(material_instance);
    return material_instance;
  }

  RenderTarget* CreateRenderTarget(const RenderTarget::Builder& builder) {
    RenderTarget* rt = Construct<RenderTarget>(builder);
    render_targets_.insert(rt);
    return rt;
  }



  static ResourceAllocator& Get() {
    static ResourceAllocator allocator;
    return allocator;
  }

  void Clear(){
    ClearRenderTarget();
    ClearMaterialInstance();
    ClearMaterial();
    ClearVertexBuffer();
    ClearIndexBuffer();
    ClearBufferObjects();
    ClearTexture();
    ClearCommonResource();
  }
 protected:
  ResourceAllocator()= default;
  ResourceAllocator(const ResourceAllocator&) = delete;
  ResourceAllocator(ResourceAllocator&&) = delete;
  ResourceAllocator& operator=(const ResourceAllocator&) = delete;
  ResourceAllocator& operator=(ResourceAllocator&&) = delete;

  template<class T, typename... ARGS>
  T* Construct(ARGS&&...args) noexcept {
    void* p = ::malloc(sizeof(T));
    return p ? new(p) T(std::forward<ARGS>(args)...) : nullptr;
  }

  void ClearCommonResource() {
    for(ResourceBase* r : common_resources_) {
      r->Destroy();
      delete r;
    }
    common_resources_.clear();
  }

  void ClearVertexBuffer() {
    for (VertexBuffer* vb : vertex_buffers_) {
      vb->Destroy();
      delete vb;
    }
    vertex_buffers_.clear();
  }

  void ClearIndexBuffer() {
    for (IndexBuffer* ib : index_buffers_) {
      ib->Destroy();
      delete ib;
    }
    index_buffers_.clear();
  }

  void ClearBufferObjects() {
    for (BufferObject* bo : buffer_objects_) {
      bo->Destroy();
      delete bo;
    }
    buffer_objects_.clear();
  }

  void ClearTexture() {
    for (Texture* tex : textures_) {
      tex->Destroy();
      delete tex;
    }
    textures_.clear();
  }

  void ClearMaterial() {
    for (Material* mat : materials_) {
      mat->Destroy();
      delete mat;
    }
    materials_.clear();
  }

  void ClearMaterialInstance() {
    for (auto iter : material_instances_) {
      for (MaterialInstance* ins : iter.second) {
        ins->Destroy();
        delete ins;
      }
      iter.second.clear();
    }
    material_instances_.clear();
  }

  void ClearRenderTarget() {
    for (RenderTarget* rt : render_targets_) {
      rt->Destroy();
      delete rt;
    }
    render_targets_.clear();
  }

 private:
  std::set<ResourceBase*> common_resources_;
  std::set<VertexBuffer*> vertex_buffers_;
  std::set<IndexBuffer*> index_buffers_;
  std::set<BufferObject*> buffer_objects_;
  std::set<Texture*> textures_;
  std::set<Material*> materials_;
  std::set<RenderTarget*> render_targets_;

  std::unordered_map<const Material*, std::set<MaterialInstance*>> material_instances_;
};



}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCEALLOCATOR_H_
