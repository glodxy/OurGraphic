//
// Created by Glodxy on 2021/11/22.
//

#include "MeshImport.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Utils/OGLogging.h"
#include "Utils/Math/PackUtils.h"
namespace our_graph::utils {
void MeshImporter::ParseFile(const std::string &file_path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(file_path,
                                               aiProcess_Triangulate |
                                               aiProcess_SortByPType |
                                               aiProcess_JoinIdenticalVertices |
                                               aiProcess_CalcTangentSpace);
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

    // 处理uv
    if (mesh->HasTextureCoords(0)) {
      dst_mesh.uvs.resize(mesh->mNumVertices);
      for (int kI = 0; kI < mesh->mNumVertices; ++kI) {
        auto uv = mesh->mTextureCoords[0][kI];
        dst_mesh.uvs[kI].x = uv.x;
        dst_mesh.uvs[kI].y = uv.y;
      }
    }

    // 处理normal与tangents,直接打包
    if (mesh->HasNormals() || mesh->HasTangentsAndBitangents()) {
      dst_mesh.normals.resize(mesh->mNumVertices);
      for (int kI = 0; kI < mesh->mNumVertices; ++kI) {
        math::Vec3 normal = {0, 0, 0};
        math::Vec3 tangents = {0, 0, 0};
        if (mesh->HasNormals()) {
          normal.x = mesh->mNormals[kI].x;
          normal.y = mesh->mNormals[kI].y;
          normal.z = mesh->mNormals[kI].z;
        }
        if (mesh->HasTangentsAndBitangents()) {
          tangents.x = mesh->mTangents[kI].x;
          tangents.y = mesh->mTangents[kI].y;
          tangents.z = mesh->mTangents[kI].z;
        }
        math::Vec4 packed_data = math::PackUtils::PackTangentFrame(normal, tangents);
        dst_mesh.normals[kI].x = packed_data.x;
        dst_mesh.normals[kI].y = packed_data.y;
        dst_mesh.normals[kI].z = packed_data.z;
        dst_mesh.normals[kI].w = packed_data.w;
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