//
// Created by Glodxy on 2021/8/31.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IRESOURCE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IRESOURCE_H_
#include <memory>
#include <cassert>
#include <string>

#include "DriverEnum.h"

namespace our_graph {

class IResource {
};

class IVertexBuffer : public IResource {
 public:
  AttributeArray attributes_ {};
  uint32_t vertex_cnt_ {};
  uint8_t buffer_cnt_ {};
  uint8_t attribute_cnt_ {};
  long padding{};
  uint8_t buffer_object_version_ {};

  IVertexBuffer() noexcept = default;
  IVertexBuffer(uint8_t buffer_count, uint8_t attribute_count,
               uint32_t vertex_count,
               const AttributeArray& attribute_array) noexcept
               : attribute_cnt_(attribute_count),
                 vertex_cnt_(vertex_count),
                 buffer_cnt_(buffer_count),
                 attributes_(attribute_array) {

  }
};

class IBufferObject : public IResource {
 public:
  uint32_t byte_cnt_{}; // 多少字节
  IBufferObject() noexcept = default;
  IBufferObject(uint32_t bytes) noexcept : byte_cnt_(bytes) {}
};

class IIndexBuffer : public IResource {
 public:
  uint32_t count_ : 27;
  uint32_t element_size_ : 5; // 每一个元素的大小

  IIndexBuffer() noexcept = default;
  IIndexBuffer(uint8_t element_size, uint32_t index_count) noexcept
              : element_size_(element_size),
                count_(index_count) {
    assert(element_size > 0 && element_size <= 16);
    assert(index_count < (1u << 27));
  }
};

class IRenderPrimitive : public IResource {
 public:
  uint32_t offset_ {};
  uint32_t min_index_ {};
  uint32_t max_index_ {};
  uint32_t count_ {};
  uint32_t max_vertex_cnt_ {};
  PrimitiveType type {PrimitiveType::TRIANGLES};
};

class IShaderProgram : public IResource {
 public:
  std::string name_;
  explicit IShaderProgram(std::string name) noexcept : name_(std::move(name)) {}

  IShaderProgram() noexcept = default;
};

class ISamplerGroup : public IResource {
 public:
  //todo: samplergroup
};

class ISwapChain : public IResource {
 public:
  virtual ~ISwapChain() = default;
};

class ITexture : public IResource {
 public:
  uint32_t width_ {};
  uint32_t height_ {};
  uint32_t depth_ {};
  SamplerType sampler_type_ {};
  uint8_t levels_ : 4;// mipmap level
  uint8_t samples_ : 4;// 采样点数
  TextureFormat format_ {};
  TextureUsage usage_ {};
  // todo： 添加stream来控制texture的传输

  ITexture() : levels_{}, samples_{} {}
  ITexture(SamplerType sampler_type, uint8_t levels, uint8_t samples,
           uint32_t width, uint32_t height, uint32_t depth,
           TextureFormat format, TextureUsage usage) noexcept :
           width_(width), height_(height), depth_(depth),
           sampler_type_(sampler_type), levels_(levels),
           samples_(samples), format_(format), usage_(usage) {

  }
};

class IRenderTarget : public IResource {
 public:
  uint32_t width_ {};
  uint32_t height_ {};
  IRenderTarget() noexcept = default;
  IRenderTarget(uint32_t width, uint32_t height) noexcept
                : width_(width), height_(height) {

  }
};

class ITimerQuery : public IResource {

};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IRESOURCE_H_
