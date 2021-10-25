//
// Created by Glodxy on 2021/10/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCEALLOCATOR_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCEALLOCATOR_H_
#include <memory>
#include <set>
#include "include_internal/ResourceBase.h"
#include "include/VertexBuffer.h"
#include "include/IndexBuffer.h"
#include "include/BufferObject.h"

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



  static ResourceAllocator& Get() {
    static ResourceAllocator allocator;
    return allocator;
  }

  void Clear(){
    ClearVertexBuffer();
    ClearIndexBuffer();
    ClearBufferObjects();
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

 private:
  std::set<ResourceBase*> common_resources_;
  std::set<VertexBuffer*> vertex_buffers_;
  std::set<IndexBuffer*> index_buffers_;
  std::set<BufferObject*> buffer_objects_;
};



}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCEALLOCATOR_H_
