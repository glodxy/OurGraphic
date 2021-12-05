//
// Created by Glodxy on 2021/12/5.
//

#ifndef OUR_GRAPHIC_UTILS_MATERIAL_UNIFORMBLOCK_H_
#define OUR_GRAPHIC_UTILS_MATERIAL_UNIFORMBLOCK_H_
#include <string>
#include <vector>
#include <unordered_map>
#include "Backend/include/DriverEnum.h"
namespace our_graph {
/**
 * 该类用于描述一个uniform块
 * */
class UniformBlock {
  using Type = UniformType;
 public:
  /**
   * 该结构体标志了一个UniformBuffer中的一个字段
   * 多个UniformInfo构成了一个UniformBuffer
   * */
  struct UniformInfo {
    std::string name; // 该uniform的名称
    uint16_t offset;  // 以4byte为单位，标识该uniform在buffer中的偏移量
    uint8_t stride; // 以4byte为单位，到下一个元素的距离（当uniform为array时)
    Type type;  // 该uniform的类型
    uint32_t size; // 元素的数量（仅当该uniform为array时有效），默认为1
    /**
     * 获取目标idx元素在整个buffer中的偏移
     * @return: 返回值为偏移的字节数
     * */
    inline size_t GetBufferOffset(size_t idx = 0) const {
      assert(idx < size);
      return (offset + stride * idx) * sizeof(uint32_t);
    }
  };

  std::string& GetName() const noexcept {
    return name_;
  }

  size_t GetSize() const noexcept {
    return size_;
  }

  std::vector<UniformInfo> const& GetUniformInfoList() const noexcept {
    return uniform_info_list_;
  }

  /**
   * 获取某个字段某个idx在整个buffer的偏移值
   * @param name:uniform的name，字段名
   * @param idx:array中的元素索引，默认为1
   * @return 当不存在时返回负数
   * */
  int64_t GetUniformOffset(const std::string& name, size_t idx) const noexcept;

  UniformInfo const* GetUniformInfo(const std::string& idx) const noexcept;

  // 判断是否存在该uniform
  bool HasUniform(const std::string& name) const noexcept {
    return info_map_.find(name) != info_map_.end();
  }

  bool IsEmpty() const noexcept {return uniform_info_list_.empty();}
 private:
  static uint8_t ComputeAlignmentForType(Type type) noexcept;
  static uint8_t ComputeStrideForType(Type type) noexcept;

  std::string name_;
  // 所有info的缓存（即所有字段的集合）
  std::vector<UniformInfo> uniform_info_list_;
  /**
   * 该map存储了uniform的name与list中的idx对应的映射关系
   * @key: uniform的name
   * @value : uniform_info_list_中的idx
   * */
  std::unordered_map<std::string, uint32_t> info_map_;
  // 以byte为单位的大小
  uint32_t size_ = 0;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_UTILS_MATERIAL_UNIFORMBLOCK_H_
