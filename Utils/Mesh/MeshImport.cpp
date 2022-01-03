//
// Created by Glodxy on 2021/11/22.
//

#include "MeshImport.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Utils/OGLogging.h"

namespace our_graph::utils {
void MeshImporter::ParseFile(const std::string &file_path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(file_path,
                                               aiProcess_Triangulate |
                                               aiProcess_SortByPType |
                                               aiProcess_JoinIdenticalVertices);
  if (!scene) {
    LOG_ERROR("MeshImporter", "Parse File {} Failed!", file_path);
    return;
  }

  // 处理mesh
  if (scene->HasMeshes()) {
    ProcessMesh(scene);
  }
}

void MeshImporter::ProcessMesh(const aiScene *scene) {
  uint32_t mesh_size = scene->mNumMeshes;
  for (int i = 0; i < mesh_size; ++i) {
    auto mesh = scene->mMeshes[i];
    Mesh dst_mesh;
    // 处理vertex
    dst_mesh.vertices.resize(mesh->mNumVertices);
    for (int kI = 0; kI < mesh->mNumVertices; ++kI) {
      auto vertex = mesh->mVertices[kI];
      dst_mesh.vertices[kI].x = vertex.x;
      dst_mesh.vertices[kI].y = vertex.y;
      dst_mesh.vertices[kI].z = vertex.z;
      dst_mesh.vertices[kI].w = 1.0f;
    }

    // 处理normal
    if (mesh->HasNormals()) {
      dst_mesh.normals.resize(mesh->mNumVertices);
      for (int kI = 0; kI < mesh->mNumVertices; ++kI) {
        auto normal = mesh->mNormals[kI];
        dst_mesh.normals[kI].x = normal.x;
        dst_mesh.normals[kI].y = normal.y;
        dst_mesh.normals[kI].z = normal.z;
        dst_mesh.normals[kI].w = 1.0f;
      }
    }

    // 处理索引
    // 此处仅处理三角形
    if (mesh->HasFaces()) {
      dst_mesh.indices.resize(mesh->mNumFaces);
      for (int kI = 0; kI < mesh->mNumFaces; ++kI) {
        if (mesh->mFaces[kI].mNumIndices != 3) {
          continue;
        }
        auto face = mesh->mFaces[kI];
        dst_mesh.indices[kI].x = face.mIndices[0];
        dst_mesh.indices[kI].y = face.mIndices[1];
        dst_mesh.indices[kI].z = face.mIndices[2];
      }
    }

    // 加入mesh缓存
    meshes_.push_back(dst_mesh);
  }
}

}