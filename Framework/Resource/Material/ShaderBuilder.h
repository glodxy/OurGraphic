//
// Created by Glodxy on 2021/12/9.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERBUILDER_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERBUILDER_H_
#include <cstddef>
namespace our_graph {
class ShaderBuilder {
 public:
  const void* GetData() const;
  size_t GetSize() const;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIAL_SHADERBUILDER_H_
