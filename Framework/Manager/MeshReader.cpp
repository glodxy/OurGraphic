//
// Created by Glodxy on 2021/11/22.
//

#include "MeshReader.h"
#include "Utils/Mesh/MeshImport.h"

namespace our_graph {
MeshReader::MeshCache MeshReader::mesh_cache_ = {};

MeshReader::MeshReader(Driver *driver) :driver_(driver) {

}

uint32_t MeshReader::GetMeshSize() const {
  return current_mesh_.size();
}

IndexBuffer * MeshReader::GetIndexBufferAt(uint32_t idx) {
  return current_mesh_[idx].index;
}

VertexBuffer * MeshReader::GetVertexBufferAt(uint32_t idx) {
  return current_mesh_[idx].vertex;
}

RenderPrimitiveHandle MeshReader::GetPrimitiveAt(uint32_t idx) {
  return current_mesh_[idx].primitive_handle;
}

void MeshReader::LoadMeshFromFile(const std::string file_name) {
  if (mesh_cache_.find(file_name) != mesh_cache_.end()) {
    current_mesh_ = mesh_cache_[file_name];
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
                            .Attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT4)
                            .Attribute(VertexAttribute::TANGENTS, 1, ElementType::FLOAT4)
                            .Build();
    // 顶点
    void* vertex_data = ::malloc(src_mesh.vertices.size() * sizeof(math::Vec4));
    memcpy(vertex_data, src_mesh.vertices.data(), src_mesh.vertices.size() * sizeof(math::Vec4));
    vertex->SetBufferAt(0, BufferDescriptor(vertex_data, src_mesh.vertices.size() * sizeof(math::Vec4), [](void* buffer, size_t size, void* user) {
      ::free(buffer);
    }));
    // 法线
    void* normal_data = ::malloc(src_mesh.normals.size() * sizeof(math::Vec4));
    memcpy(normal_data, src_mesh.normals.data(), src_mesh.normals.size() * sizeof(math::Vec4));
    vertex->SetBufferAt(1, BufferDescriptor(normal_data, src_mesh.normals.size() * sizeof(math::Vec4), [](void* buffer, size_t size, void* user) {
      ::free(buffer);
    }));
    // 索引
    IndexBuffer* index = IndexBuffer::Builder(driver_)
                          .BufferType(IndexBuffer::IndexType::UINT)
                          .IndexCount(src_mesh.indices.size() * 3)
                          .Build();
    void* index_data = ::malloc(src_mesh.indices.size() * sizeof(math::Vec3ui));
    memcpy(index_data, src_mesh.indices.data(), src_mesh.indices.size() * sizeof(math::Vec3ui));
    index->SetBuffer(BufferDescriptor(index_data, src_mesh.indices.size() * sizeof(math::Vec3ui), [](void*buffer, size_t size, void* user) {
      ::free(buffer);
    }));

    auto handle = driver_->CreateRenderPrimitive();
    driver_->SetRenderPrimitiveBuffer(handle, vertex->GetHandle(), index->GetHandle());
    driver_->SetRenderPrimitiveRange(handle, PrimitiveType::TRIANGLES, 0, 0, 0, src_mesh.indices.size() * 3);
    Mesh mesh {
      .vertex = vertex,
      .index = index,
      .primitive_handle = std::move(handle)
    };
    mesh_list.push_back(mesh);
  }
  mesh_cache_[file_name] = mesh_list;
  current_mesh_ = mesh_list;
}
}  // namespace our_graph