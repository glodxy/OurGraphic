//
// Created by Glodxy on 2021/10/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_INDEXBUFFER_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_INDEXBUFFER_H_
#include <bitset>
#include "base/BuilderBase.h"
#include "Backend/include/Handle.h"
#include "Backend/include/BufferDescriptor.h"
#include "Backend/include/DriverEnum.h"
#include "Backend/include/Driver.h"
#include "include_internal/ResourceBase.h"
namespace our_graph {
class IndexBuffer : public ResourceBase {
  friend class ResourceAllocator;
  struct Detail;

 public:
  enum class IndexType : uint8_t {
    USHORT = uint8_t (ElementType::USHORT),
    UINT = uint8_t (ElementType::UINT),
  };

  class Builder : public BuilderBase<Detail> {
    friend class IndexBuffer;
   public:
    Builder(Driver* driver) noexcept;
    Builder(const Builder& rhs) noexcept;
    Builder(Builder&& rhs) noexcept;
    ~Builder() noexcept;
    Builder& operator=(const Builder& rhs) noexcept;
    Builder& operator=(Builder&& rhs) noexcept;

    Builder& IndexCount(uint8_t index_count) noexcept;

    /**
     * Buffer的类型：存储在里面的数据类型
     * */
    Builder& BufferType(IndexType index_type) noexcept;

    IndexBuffer* Build();
  };

  size_t GetIndexCount() const {return index_count_;}

  void SetBuffer(BufferDescriptor&& data, uint32_t byte_offset = 0);

  IndexBufferHandle GetHandle() const {return handle_;}

  void Destroy() override;

  ~IndexBuffer() override = default;
 protected:
  // 构造函数
  IndexBuffer(const Builder& builder);
 private:
  IndexBufferHandle handle_;
  uint32_t index_count_;
  Driver* driver_;
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_INDEXBUFFER_H_
