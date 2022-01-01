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
  void AppendData(const void* data, size_t size){}

 private:
  std::vector<uint8_t> data_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERBUILDER_H_
