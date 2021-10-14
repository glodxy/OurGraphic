//
// Created by Glodxy on 2021/10/6.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_DRIVER_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_DRIVER_H_
#include "PixelBufferDescriptor.h"
#include "BufferDescriptor.h"
#include "DriverEnum.h"
#include "../include_internal/IPlatform.h"
#include "../include_internal/IResource.h"
#include "Handle.h"
#include "PipelineState.h"
#include "SamplerGroup.h"
#include "Program.h"

#include "glm/glm.hpp"

#include <cstdint>
#include <vector>

namespace our_graph {

struct TextureSwizzle {};
struct MRT {};
struct TargetBufferInfo {};


struct FrameScheduledCallback{};
struct FrameCompeletedCallback {};

class DriverApi {
  using byte = glm::i8;
  using byte2 = glm::i8vec2;
  using byte3 = glm::i8vec3;
  using byte4 = glm::i8vec4;

  using ubyte = glm::u8;
  using ubyte2 = glm::u8vec2;
  using ubyte3 = glm::u8vec3;
  using ubyte4 = glm::u8vec4;

  using short2 = glm::vec<2, int16_t, glm::defaultp>;
  using short3 = glm::vec<3, int16_t, glm::defaultp>;
  using short4 = glm::vec<4, int16_t, glm::defaultp>;

  using ushort2 = glm::vec<2, uint16_t, glm::defaultp>;
  using ushort3 = glm::vec<3, uint16_t, glm::defaultp>;
  using ushort4 = glm::vec<4, uint16_t, glm::defaultp>;

  using float2 = glm::fvec2;
  using float3 = glm::fvec3;
  using float4 = glm::fvec4;

  using half = glm::uint16_t;
  using half2 = glm::vec<2, uint16_t, glm::defaultp>;
  using half3 = glm::vec<3, uint16_t, glm::defaultp>;
  using half4 = glm::vec<4, uint16_t, glm::defaultp>;
 public:
  static size_t GetElementTypeSize(ElementType type) noexcept {
    switch (type) {
      case ElementType::BYTE:     return sizeof(byte);
      case ElementType::BYTE2:    return sizeof(byte2);
      case ElementType::BYTE3:    return sizeof(byte3);
      case ElementType::BYTE4:    return sizeof(byte4);
      case ElementType::UBYTE:    return sizeof(ubyte);
      case ElementType::UBYTE2:   return sizeof(ubyte2);
      case ElementType::UBYTE3:   return sizeof(ubyte3);
      case ElementType::UBYTE4:   return sizeof(ubyte4);
      case ElementType::SHORT:    return sizeof(int16_t);
      case ElementType::SHORT2:   return sizeof(short2);
      case ElementType::SHORT3:   return sizeof(short3);
      case ElementType::SHORT4:   return sizeof(short4);
      case ElementType::USHORT:   return sizeof(uint16_t);
      case ElementType::USHORT2:  return sizeof(ushort2);
      case ElementType::USHORT3:  return sizeof(ushort3);
      case ElementType::USHORT4:  return sizeof(ushort4);
      case ElementType::INT:      return sizeof(int32_t);
      case ElementType::UINT:     return sizeof(uint32_t);
      case ElementType::FLOAT:    return sizeof(float);
      case ElementType::FLOAT2:   return sizeof(float2);
      case ElementType::FLOAT3:   return sizeof(float3);
      case ElementType::FLOAT4:   return sizeof(float4);
      case ElementType::HALF:     return sizeof(half);
      case ElementType::HALF2:    return sizeof(half2);
      case ElementType::HALF3:    return sizeof(half3);
      case ElementType::HALF4:    return sizeof(half4);
    }
  }
 public:
  virtual void Init(std::unique_ptr<IPlatform> platform) {}

  /**
   * 清除无用的资源
   * */
  virtual void Clear() {}


  virtual void Tick() {}
//每帧的api////////////////////////////////////////
  virtual void BeginFrame(int64_t time_ns,
                          uint32_t frame_id) { }
  // 设置每帧调度时的回调
  virtual void SetFrameScheduledCallback(
      SwapChainHandle handle,
      FrameScheduledCallback callback,
      void* user) {}

  // 设置每帧结束时的回调
  virtual void SetFrameCompleteCallback(
      SwapChainHandle handle,
      FrameCompeletedCallback callback,
      void* user) {}

  virtual void EndFrame(uint32_t frame_id) {}
  // 提交至Driver层
  virtual void Flush() {}
  // 等待每帧执行完
  virtual void Finish() {}
//////////////////////////////////////////////////
/////////////////创建资源//////////////////////////
  // 创建顶点缓冲区
  virtual VertexBufferHandle CreateVertexBuffer(
      uint8_t buffer_cnt,
      uint8_t attribute_cnt,
      uint32_t vertex_cnt,
      AttributeArray attributes) {
    return VertexBufferHandle(HandleBase::NULL_HANDLE);
  }

  // 创建索引缓冲区
  virtual IndexBufferHandle CreateIndexBuffer(
      ElementType element_type,
      uint32_t index_cnt,
      BufferUsage usage) {
    return IndexBufferHandle(HandleBase::NULL_HANDLE);
  }

  // 创建普通buffer
  virtual BufferObjectHandle CreateBufferObject(
      uint32_t bytes,
      BufferObjectBinding binding_type,
      BufferUsage usage) {
    return BufferObjectHandle(HandleBase::NULL_HANDLE);
  }

  // 创建纹理
  virtual TextureHandle CreateTexture(
      SamplerType target,
      uint8_t levels,
      TextureFormat format,
      uint8_t samples,
      uint32_t width,
      uint32_t height,
      uint32_t depth,
      TextureUsage usage) {
    return TextureHandle(HandleBase::NULL_HANDLE);
  }

  // 创建swizzle布局的纹理
  virtual TextureHandle CreateTextureSwizzled(
      SamplerType target,
      uint8_t levels,
      TextureFormat format,
      uint8_t samples,
      uint32_t width,
      uint32_t height,
      uint32_t depth,
      TextureUsage usage,
      TextureSwizzle r,
      TextureSwizzle g,
      TextureSwizzle b,
      TextureSwizzle a) {
    return TextureHandle(HandleBase::NULL_HANDLE);
  }

  // 导入纹理
  virtual TextureHandle ImportTexture(
      intptr_t id,
      SamplerType target,
      uint8_t levels,
      TextureFormat format,
      uint8_t samples,
      uint32_t width,
      uint32_t height,
      uint32_t depth,
      TextureUsage usage) {
    return TextureHandle(HandleBase::NULL_HANDLE);
  }

  virtual SamplerGroupHandle CreateSamplerGroup(uint32_t size) {
    return SamplerGroupHandle(HandleBase::NULL_HANDLE);
  }

  virtual RenderPrimitiveHandle CreateRenderPrimitive() {
    return RenderPrimitiveHandle(HandleBase::NULL_HANDLE);
  }

  virtual ShaderHandle CreateShader(Program&& shaders) {
    return ShaderHandle(HandleBase::NULL_HANDLE);
  }

  // 创建默认的渲染目标
  virtual RenderTargetHandle CreateDefaultRenderTarget() {
    return RenderTargetHandle(HandleBase::NULL_HANDLE);
  }

  /**
   * 创建一个RenderTarget，可以通过参数设置是否用于深度或模板
   * */
  virtual RenderTargetHandle CreateRenderTarget(
      TargetBufferFlags target_flags,
      uint32_t width,
      uint32_t height,
      uint8_t samples,
      MRT color,
      TargetBufferInfo depth,
      TargetBufferInfo stencil) {
    return RenderTargetHandle(HandleBase::NULL_HANDLE);
  }

  /**
   * 创建交换链
   * @param native_window: 窗口句柄
   * */
  virtual SwapChainHandle CreateSwapChain(
      void* native_window, uint64_t flags) {
    return SwapChainHandle(HandleBase::NULL_HANDLE);
  }

  virtual TimerQueryHandle CreateTimerQuery() {
    return TimerQueryHandle(HandleBase::NULL_HANDLE);
  }
///////////////////////////////////////////////////////
////////////销毁资源/////////////////////////////////////
  virtual void DestroyVertexBuffer(VertexBufferHandle handle) {}
  virtual void DestroyIndexBuffer(IndexBufferHandle handle) {}
  virtual void DestroyBufferObject(BufferObjectHandle handle) {}
  virtual void DestroyRenderPrimitive(RenderPrimitiveHandle handle) {}
  virtual void DestroyShader(ShaderHandle handle) {}
  virtual void DestroySamplerGroup(SamplerGroupHandle handle) {}
  virtual void DestroyTexture(TextureHandle handle) {}
  virtual void DestroyRenderTarget(RenderTargetHandle handle) {}
  virtual void DestroySwapChain(SwapChainHandle handle) {}
  virtual void DestroyTimerQuery(TimerQueryHandle handle) {}
//////////////////////////////////////////////////////////
/////////////更新Driver数据/////////////////////////////////
  // 设置顶点缓冲区的数据
  virtual void SetVertexBufferObject(
      VertexBufferHandle handle,
      uint32_t index,
      BufferObjectHandle buffer_handle) {

  }

  virtual void UpdateIndexBuffer(
      IndexBufferHandle handle,
      BufferDescriptor&& data,
      uint32_t byte_offset) {

  }

  virtual void UpdateBufferObject(
      BufferObjectHandle handle,
      BufferDescriptor&& data,
      uint32_t byte_offset) {

  }

  virtual void UpdateSamplerGroup(
      SamplerGroupHandle handle,
      SamplerGroup&& sampler_group) {}

  virtual void Update2DImage(
      TextureHandle handle,
      uint32_t level,
      uint32_t x_offset,
      uint32_t y_offset,
      uint32_t width,
      uint32_t height,
      PixelBufferDescriptor&& data) {}

  // 设置纹理的最大最小mipmap级别
  virtual void SetMinMaxLevels(
      TextureHandle handle,
      uint32_t min_level,
      uint32_t max_level) {}

  virtual void Update3DImage(
      TextureHandle handle,
      uint32_t level,
      uint32_t x_offset,
      uint32_t y_offset,
      uint32_t z_offset,
      uint32_t width,
      uint32_t height,
      uint32_t depth,
      PixelBufferDescriptor&& data) {}

  virtual void UpdateCubeImage(
      TextureHandle handle,
      uint32_t level,
      PixelBufferDescriptor&& data,
      FaceOffsets face_offsets) {}

  virtual void GenerateMipmaps(TextureHandle handle) {}

  virtual void SetExternalImage(
      TextureHandle handle,
      void* image) {}

  virtual void BeginRenderPass(
      RenderTargetHandle handle,
      const RenderPassParams& params) {}

  virtual void EndRenderPass() {}

  virtual void NextSubPass() {}

  virtual void SetRenderPrimitiveBuffer(
      RenderPrimitiveHandle handle,
      VertexBufferHandle vertex,
      IndexBufferHandle index) {}

  virtual void SetRenderPrimitiveRange(
      RenderPrimitiveHandle handle,
      PrimitiveType type,
      uint32_t offset,
      uint32_t min_idx,
      uint32_t max_idx,
      uint32_t cnt) {}

  virtual void beginTimerQuery(TimerQueryHandle handle) {}
  virtual void EndTimerQuery(TimerQueryHandle handle) {}
///////////////////////////////////////////////////////
//////////////////交换链///////////////////////////////
  virtual void MakeCurrent(
      SwapChainHandle draw,
      SwapChainHandle read) {}

  virtual void Commit(SwapChainHandle handle) {}
////////////////////////////////////////////////////////
///////////////设置渲染状态////////////////////////////
  virtual void BindUniformBuffer(
      uint32_t idx,
      BufferObjectHandle handle) {}

  virtual void BindUniformBufferRange(
      uint32_t idx,
      BufferObjectHandle handle,
      uint32_t offset,
      uint32_t size) {}

  virtual void BindSamplers(
      uint32_t idx,
      SamplerGroupHandle handle) {}

  virtual void StartCapture() {}
  virtual void EndCapture() {}
////////////////////////////////////////////////////////

/////////////渲染操作///////////////////////////////////
  // 将一个render target的数据贴到另一个上
  virtual void Blit(
      TargetBufferInfo buffers,
      RenderTargetHandle dst,
      Viewport dst_rect,
      RenderTargetHandle src,
      Viewport src_rect,
      SamplerMagFilter filter) {}

  virtual void Draw(
      PipelineState state,
      RenderPrimitiveHandle handle) {}
/////////////////////////////////////////////////////////

////////同步接口//////////////////////////////////////////
  virtual void Terminate() {}
  virtual bool IsTextureFormatSupported(TextureFormat format) {
    return true;
  }
  virtual bool IsTextureSwizzleSupported() {
    return true;
  }
  virtual bool IsTextureFormatMipmappable(TextureFormat format) {
    return true;
  }
  virtual bool IsRenderTargetFormatSupported(TextureFormat format) {
    return true;
  }
  virtual bool IsFrameBufferFetchSupported() {
    return true;
  }
  virtual bool IsFrameTimeSupported() {
    return true;
  }
  virtual uint8_t GetMaxDrawBuffers() {
    return 0;
  }
  virtual std::vector<float> GetClipSpaceParam() {
    return {0, 0};
  }
  virtual bool CanGenerateMipmaps() {
    return true;
  }
  virtual void SetupExternalImage(void* image) {}
  virtual void CancelExternalImage(void* image) {}
  virtual bool GetTimerQueryValue(TimerQueryHandle handle, uint64_t* elapsed_time) {
    return true;
  }

};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_DRIVER_H_
