//
// Created by chaytian on 2021/11/2.
//

#include "SoftDriver.h"
#include "Backend/include_internal/Dispatcher.h"
#include "SoftSwapChain.h"
#include "SoftPipeline.h"
#include "Backend/Soft/Resource/SoftRenderTarget.h"
#include "Backend/Soft/Resource/SoftRenderPrimitive.h"
#include "Backend/Soft/Resource/SoftBuffer.h"
namespace our_graph {



SoftDriver::SoftDriver(SDL_Window *window) : DriverApi() {
  dispatcher_ = new Dispatcher<SoftDriver>();
  SoftContext::Get().window_ = window;
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  SoftContext::Get().window_width_ = w;
  SoftContext::Get().window_height_ = h;

  SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SoftContext::Get().renderer_ = renderer;

  pipeline_ = std::make_unique<SoftPipeline>();
}

SoftDriver::~SoftDriver() {
  SDL_DestroyRenderer(SoftContext::Get().renderer_);
  SoftContext::Get().renderer_ = nullptr;
  SoftContext::Get().current_tex_ = nullptr;
  delete dispatcher_;
}

SwapChainHandle SoftDriver::CreateSwapChainS() {
  return AllocHandle<SoftSwapChain>();
}

void SoftDriver::CreateSwapChainR(SwapChainHandle handle, void *native_window, uint64_t flags) {
  Construct<SoftSwapChain>(handle);
}

RenderTargetHandle SoftDriver::CreateDefaultRenderTargetS() {
  return AllocHandle<SoftRenderTarget>();
}

void SoftDriver::CreateDefaultRenderTargetR(RenderTargetHandle handle) {
  Construct<SoftRenderTarget>(handle);
}

RenderPrimitiveHandle SoftDriver::CreateRenderPrimitiveS() {
  return AllocHandle<SoftRenderPrimitive>();
}

void SoftDriver::CreateRenderPrimitiveR(RenderPrimitiveHandle handle) {
  Construct<SoftRenderPrimitive>(handle);
}
void SoftDriver::SetRenderPrimitiveBuffer(RenderPrimitiveHandle handle,
                                          VertexBufferHandle vertex,
                                          IndexBufferHandle index) {
  auto rp = HandleCast<SoftRenderPrimitive*>(handle);
  auto vertex_buffer = HandleCast<SoftVertexBuffer*>(vertex);
  rp->BindVertex(vertex_buffer);
  //todo:index
}

BufferObjectHandle SoftDriver::CreateBufferObjectS() {
  return AllocHandle<SoftBuffer>();
}

void SoftDriver::CreateBufferObjectR(BufferObjectHandle handle,
                                     uint32_t bytes,
                                     BufferObjectBinding binding_type,
                                     BufferUsage usage) {
  Construct<SoftBuffer>(handle, bytes);
}

void SoftDriver::UpdateBufferObject(BufferObjectHandle handle, BufferDescriptor &&data, uint32_t byte_offset) {
  auto buffer = HandleCast<SoftBuffer*>(handle);
  assert(buffer->byte_cnt_ == data.size_);
  memcpy(buffer->buffer, data.buffer_, data.size_);
}

void SoftDriver::DestroyBufferObject(BufferObjectHandle handle) {
  Destruct<SoftBuffer>(handle);
}

///vertex//////////////////////////////////
VertexBufferHandle SoftDriver::CreateVertexBufferS() {
  return AllocHandle<SoftVertexBuffer>();
}
void SoftDriver::CreateVertexBufferR(VertexBufferHandle handle,
                                     uint8_t buffer_cnt,
                                     uint8_t attribute_cnt,
                                     uint32_t vertex_cnt,
                                     AttributeArray attributes) {
  Construct<SoftVertexBuffer>(handle, buffer_cnt, attribute_cnt, vertex_cnt, attributes);
}
void SoftDriver::SetVertexBufferObject(VertexBufferHandle handle, uint32_t index, BufferObjectHandle buffer_handle) {
  auto buffer = HandleCast<SoftBuffer*>(buffer_handle);
  auto vertex_buffer = HandleCast<SoftVertexBuffer*>(handle);
  vertex_buffer->buffers_[index] = buffer;
}

////index//////////////////////////////////
IndexBufferHandle SoftDriver::CreateIndexBufferS() {
  return AllocHandle<SoftIndexBuffer>();
}

void SoftDriver::CreateIndexBufferR(IndexBufferHandle handle,
                                    ElementType element_type,
                                    uint32_t index_cnt,
                                    BufferUsage usage) {
  Construct<SoftIndexBuffer>(handle, element_type, index_cnt);
}

void SoftDriver::UpdateIndexBuffer(IndexBufferHandle handle, BufferDescriptor &&data, uint32_t byte_offset) {
  auto index_buffer = HandleCast<SoftIndexBuffer*>(handle);
  assert(index_buffer->buffer_->byte_cnt_ == data.size_);
  memcpy(index_buffer->buffer_->buffer, data.buffer_, data.size_);
}
////////////////////////////////////
void SoftDriver::BeginRenderPass(RenderTargetHandle handle, const RenderPassParams &params) {
  auto rt = HandleCast<SoftRenderTarget*>(handle);
  current_rt_ = rt;
  // 开始渲染前，先清除
  current_rt_->ClearColor();
  current_rt_->ClearDepth();
}

/**
 * 在此处将当前render target的颜色提交到surface
 * */
void SoftDriver::EndRenderPass() {
  // 非离屏render target，则需提交至swapchain的纹理
  if (!current_rt_->IsOffscreen()) {
    SDL_Texture* current_texture = SoftContext::Get().current_tex_;
    void* tex;
    int pitch;
    SDL_LockTexture(current_texture, nullptr, &tex, &pitch);
    uint32_t * pixels = (uint32_t * const) current_rt_->GetData();
    // 获取像素的个数
    uint32_t size = current_rt_->GetSize();
    memcpy(tex, pixels, sizeof(uint32_t) * size);
//    for (int i = 0; i < size; ++i) {
//      ((uint32_t*)tex)[i] = pixels[i];
//    }
    LOG_INFO("SoftPipeline", "PixelBlit {} pixels", size);
    SDL_UnlockTexture(current_texture);
  }
}

void SoftDriver::Commit(SwapChainHandle handle) {
  auto swap_chain = HandleCast<SoftSwapChain*>(handle);
  swap_chain->CommitAndAcquireNext();
  // 判断resize
  int w, h;
  SDL_Window* window = SoftContext::Get().window_;
  SDL_GetWindowSize(window, &w, &h);
  SoftContext::Get().window_width_ = w;
  SoftContext::Get().window_height_ = h;
  if (w != cur_width_ || h != cur_height_) {
    swap_chain->Resize();
  }
  cur_width_ = w;
  cur_height_ = h;
  SoftContext::Get().current_tex_ = swap_chain->GetCurrent();
}

void SoftDriver::Draw(PipelineState state, RenderPrimitiveHandle handle) {
  auto primitive = HandleCast<SoftRenderPrimitive*>(handle);
  SoftVertexBuffer* vertex = primitive->GetVertexBuffer();
  size_t cnt = vertex->vertex_cnt_;
  Vec3 * position = (Vec3*)vertex->buffers_[VertexAttribute::POSITION]->buffer;
  Vec3 * normal = (Vec3*)vertex->buffers_[VertexAttribute::TANGENTS]->buffer;
  // todo:优化backend的vertex缓存
  Vertex* data = new Vertex[cnt];
  // 设置坐标
  for (int i = 0; i < cnt; ++i) {
    data[i].position = position[i];
    data[i].data.world_normal = normal[i];
  }
  pipeline_->Execute(data, cnt, current_rt_);
  delete[] data;
}

}