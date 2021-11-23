//
// Created by Glodxy on 2021/11/22.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_COMPONENT_MESHREADER_H_
#define OUR_GRAPHIC_FRAMEWORK_COMPONENT_MESHREADER_H_
#include <map>
#include <string>
#include "Framework/Backend/include/Driver.h"
#include "Framework/Resource/include/VertexBuffer.h"
#include "Framework/Resource/include/IndexBuffer.h"
namespace our_graph {
/**
 * 该类只由render system调用
 * renderable的Mesh中仅存储文件名等指示参数
 * 实际的Mesh在Render System的逻辑中生成并缓存
 * */
class MeshReader {
 public:
  explicit MeshReader(Driver* driver);


  // 从文件加载Mesh
  void LoadMeshFromFile(const std::string file_name);

  uint32_t GetMeshSize() const;
  VertexBuffer* GetVertexBufferAt(uint32_t idx);
  IndexBuffer* GetIndexBufferAt(uint32_t idx);
  RenderPrimitiveHandle GetPrimitiveAt(uint32_t idx);
 private:
  Driver* driver_ {nullptr};

  /**
   * 该Mesh不由Entity直接持有
   * 该Mesh只用于该类的内部缓存
   * */
  struct Mesh {
    VertexBuffer* vertex;
    IndexBuffer* index;
    RenderPrimitiveHandle primitive_handle;
  };
  std::vector<Mesh> current_mesh_; // 当前解析得到的mesh
  // 每个file可能拥有多个mesh
  using MeshCache = std::map<std::string, std::vector<Mesh>>;
  // 该cache由所有reader共同持有
  // 方便reader共享解析的结果
  static MeshCache mesh_cache_; // mesh的cache
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_MESHREADER_H_
