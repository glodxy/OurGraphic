//
// Created by Glodxy on 2021/11/22.
//

#include "MeshReader.h"
#include "Utils/Mesh/MeshImport.h"

namespace our_graph {
MeshReader::MeshCache MeshReader::mesh_cache_ = {};

MeshReader::MeshReader(Driver *driver) :driver_(driver) {

}

void MeshReader::LoadMeshFromFile(const std::string file_name) {
  if (mesh_cache_.find(file_name) != mesh_cache_.end()) {
    return;
  }
  utils::MeshImporter importer;
  importer.ParseFile(file_name);
  std::vector<Mesh> mesh_list;
  // 处理单个mesh
  for (int kI = 0; kI < importer.meshes_.size(); ++kI) {
    const auto& src_mesh = importer.meshes_[kI];
    VertexBuffer* vertex = VertexBuffer::Builder(driver_)
                            .BufferCount(2)
                            .VertexCount(src_mesh.vertices.size())
                            .Attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT3)
                            .Attribute(VertexAttribute::CUSTOM0, 1, ElementType::FLOAT3)
                            .Build();
    // 顶点
    vertex->SetBufferAt(0, BufferDescriptor(src_mesh.vertices.data(), src_mesh.vertices.size() * sizeof(math::Vec3)));
    // 法线
    vertex->SetBufferAt(1, BufferDescriptor(src_mesh.normals.data(), src_mesh.normals.size() * sizeof(math::Vec3)));

    // 索引
    IndexBuffer* index = IndexBuffer::Builder(driver_)
                          .BufferType(IndexBuffer::IndexType::UINT)
                          .IndexCount(src_mesh.indices.size() * 3)
                          .Build();
    index->SetBuffer(BufferDescriptor(src_mesh.indices.data(), src_mesh.indices.size() * sizeof(math::Vec3i)));
    Mesh mesh {
      .vertex = vertex,
      .index = index
    };
    mesh_list.push_back(mesh);
  }
  mesh_cache_[file_name] = mesh_list;
}
}  // namespace our_graph