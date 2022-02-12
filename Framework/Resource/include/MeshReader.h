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
 * todo:MeshCache
 * */
class MeshReader {
  struct MeshKey {
    std::string name;
    explicit MeshKey(std::string n) : name(std::move(n)) {}

    bool operator<(const MeshKey& r) const {
      return name < r.name;
    }
  };
 public:
  static void Init(Driver* driver);


  // 从文件加载Mesh
  static void LoadMeshFromFile(const std::string file_name);

  static uint32_t GetMeshSize();
  static VertexBuffer* GetVertexBufferAt(uint32_t idx);
  static IndexBuffer* GetIndexBufferAt(uint32_t idx);
  static RenderPrimitiveHandle GetPrimitiveAt(uint32_t idx);

  /**
   * 获取屏幕空间的primitive，即一个矩形
   * */
  static RenderPrimitiveHandle GetQuadPrimitive();

  /**
   * 获取cubemap的primitive，即立方体
   * */
  static RenderPrimitiveHandle GetCubemapPrimitive();
 private:
  static void InitQuadPrimitive();
  static void InitCubemapPrimitive();


  static Driver* driver_;

  /**
   * 该Mesh不由Entity直接持有
   * 该Mesh只用于该类的内部缓存
   * */
  struct Mesh {
    VertexBuffer* vertex;
    IndexBuffer* index;
    RenderPrimitiveHandle primitive_handle;
  };
  static std::vector<Mesh> current_mesh_; // 当前解析得到的mesh
  // 每个file可能拥有多个mesh
  using MeshCache = std::map<MeshKey, std::vector<Mesh>>;
  // 该cache由所有reader共同持有
  // 方便reader共享解析的结果
  static MeshCache mesh_cache_; // mesh的cache
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_COMPONENT_MESHREADER_H_
