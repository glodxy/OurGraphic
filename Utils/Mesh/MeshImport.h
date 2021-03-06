//
// Created by Glodxy on 2021/11/22.
//

#ifndef OUR_GRAPHIC_UTILS_MESH_MESHIMPORT_H_
#define OUR_GRAPHIC_UTILS_MESH_MESHIMPORT_H_
#include <string>
#include <vector>
#include "Utils/Math/Math.h"
#include "assimp/scene.h"
namespace our_graph::utils {
class MeshImporter {
 public:
  enum FileType {
    UNKNOWN,
    OBJ,
    FBX
  };

  void ParseFile(const std::string& file_path);

  struct Mesh {
    std::vector<math::Vec4> vertices; // 顶点
    std::vector<math::Vec4> normals; // 法线
    std::vector<math::Vec3ui> indices; // 索引
    std::vector<math::Vec2> uvs; // uv坐标
  };

  std::vector<Mesh> meshes_; // mesh 缓存

 private:
  void ProcessMesh(const aiScene* scene);

  FileType GetFileType(const std::string& file_name);
  // 模型文件的类型
  FileType file_type_;
};
}
#endif //OUR_GRAPHIC_UTILS_MESH_MESHIMPORT_H_
