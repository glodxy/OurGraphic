//
// Created by Glodxy on 2021/12/5.
//

#ifndef OUR_GRAPHIC_UTILS_MATERIAL_SAMPLERBLOCK_H_
#define OUR_GRAPHIC_UTILS_MATERIAL_SAMPLERBLOCK_H_
#include <string>
#include <unordered_map>
#include <vector>
#include "Backend/include/DriverEnum.h"
namespace our_graph{
class SamplerBlock {
 public:
  using Type = SamplerType;
  using Format = SamplerFormat;
  using SamplerParams = SamplerParams;

  SamplerBlock() = default;
  SamplerBlock(const SamplerBlock&) = default;
  SamplerBlock& operator=(const SamplerBlock&) = default;
  ~SamplerBlock() = default;
 public:
  class Builder {
    friend class SamplerBlock;
   public:
    Builder() = default;
    Builder(const Builder&) = default;
    ~Builder() = default;

    Builder& Name(const std::string& name);

    Builder& Add(const std::string& name, Type type,
                 Format format, bool multisample = false);

    SamplerBlock Build();

   private:
    struct Entry {
      Entry(const std::string& name, Type type, Format format, bool multi_sample) noexcept :
         name_(std::move(name)), type_(type), format_(format),
         multi_sample_(multi_sample) {

      }
      std::string name_;
      Type type_;
      Format format_;
      bool multi_sample_;
    };

    std::string name_;
    std::vector<Entry> entries_;
  };
  struct SamplerInfo {
    SamplerInfo() noexcept = default;
    SamplerInfo(const std::string& name, uint8_t offset, Type type,
                Format format, bool multi_sample) noexcept
                : name(name), offset(offset),
                type(type), format(format), multi_sample(multi_sample) {

    }
    std::string name;  // 采样器的名称
    uint8_t offset;  // 该sampler在buffer中的偏移
    Type type;  // sampler类型
    Format format;  // sampler元素格式
    bool multi_sample;  // 多重采样
  };

  std::string GetName() const noexcept {
    return name_;
  }

  size_t GetSize() const noexcept {
    return size_;
  }

  std::vector<SamplerInfo> const& GetSamplerInfoList() const noexcept {
    return sampler_info_list_;
  }

  SamplerInfo const * GetSamplerInfo(const std::string& name) const;

  bool HasSampler(const std::string& name) const noexcept {
    return info_map_.find(name) != info_map_.end();
  }

  bool IsEmpty() const noexcept {
    return sampler_info_list_.empty();
  }

  /**
   * 根据所属的sampler group以及该sampler的名称获取对应的uniform名称
   * */
  static std::string GetUniformName(const std::string& group,
                                    const std::string& sampler) noexcept;

 private:
  SamplerBlock(const Builder& builder);

 private:
  std::string name_;
  // 所有sampler的集合
  std::vector<SamplerInfo> sampler_info_list_;
  // 名称与idx的映射
  std::unordered_map<std::string, uint32_t> info_map_;
  // sampler的数量
  uint32_t size_ = 0;
};
}
#endif //OUR_GRAPHIC_UTILS_MATERIAL_SAMPLERBLOCK_H_
