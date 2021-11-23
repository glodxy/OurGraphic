//
// Created by Glodxy on 2021/11/22.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_RENDERABLE_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_RENDERABLE_H_
#include <memory>
#include <string>
#include "ComponentBase.h"
namespace our_graph {
/**
 * 该组件代表该entity会被用于渲染，用于设置一系列渲染参数
 * 在render system中进行实际的渲染设置与绘制
 * */
class Renderable : public ComponentBase,
 public std::enable_shared_from_this<Renderable> {

 public:
  explicit Renderable(uint32_t id);
  ~Renderable() override;
  SystemID GetSystemID() const override;

  uint32_t GetComponentType() const override;
 public:
  struct MeshInfo {
    std::string mesh_name;
    uint32_t instance_cnt; // 实例数
  };

  MeshInfo GetMeshInfo() const;
 protected:
  void Init() override;
  MeshInfo mesh_info_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_RENDERABLE_H_
