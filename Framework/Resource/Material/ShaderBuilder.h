//
// Created by Glodxy on 2021/12/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERBUILDER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERBUILDER_H_
#include <cstddef>
#include <vector>
namespace our_graph {
class ShaderBuilder {
 public:
  const void* GetData() const{return data_.data();}
  size_t GetSize() const{return data_.size();}
  void AppendData(const void* data, size_t size){
    std::vector<uint8_t> origin = data_;
    data_.resize(origin.size() + size);
    memcpy(data_.data(), origin.data(), origin.size());
    memcpy(data_.data() + origin.size() + origin.size(), data, size);
  }

 private:
  std::vector<uint8_t> data_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERBUILDER_H_
