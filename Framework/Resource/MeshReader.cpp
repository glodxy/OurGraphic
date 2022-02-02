//
// Created by Glodxy on 2021/11/22.
//

#include "Framework/Resource/include/MeshReader.h"
#include "Utils/Mesh/MeshImport.h"
namespace {
static const std::string kDefaultQuadKey = "DEFAULT_QUAD";
struct QuadVertex {
  our_graph::math::Vec2 position;
  our_graph::math::Vec2 uv;
};

static const QuadVertex QUAD_VERTICES[4] = {
    {{-1, -1}, {0, 1}},
    {{ 1, -1}, {1, 1}},
    {{-1,  1}, {0, 0}},
    {{ 1,  1}, {1, 0}},
};

static constexpr uint16_t QUAD_INDICES[6] = {
    0, 1, 2,
    3, 1, 2,
};


}

namespace our_graph {
MeshReader::MeshCache MeshReader::mesh_cache_ = {};
Driver* MeshReader::driver_ = nullptr;
std::vector<MeshReader::Mesh> MeshReader::current_mesh_ = {};

void MeshReader::Init(Driver *driver) {
  driver_= driver;
  InitQuadPrimitive();
}

void MeshReader::InitQuadPrimitive() {
  VertexBuffer* quad_vertex = VertexBuffer::Builder(driver_)
      .VertexCount(4)
      .BufferCount(1)
      .Attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT2, 0, 16)
      .Attribute(VertexAttribute::UV0, 0, ElementType::FLOAT2, 8, 16)
      .Build();

  quad_vertex->SetBufferAt(0, BufferDescriptor(QUAD_VERTICES, 64, nullptr));

  IndexBuffer* quad_index = IndexBuffer::Builder(driver_)
      .IndexCount(6)
      .BufferType(IndexBuffer::IndexType::USHORT)
      .Build();
  quad_index->SetBuffer(BufferDescriptor(QUAD_INDICES, 12, nullptr));

  auto handle = driver_->CreateRenderPrimitive();
  driver_->SetRenderPrimitiveBuffer(handle, quad_vertex->GetHandle(), quad_index->GetHandle());
  driver_->SetRenderPrimitiveRange(handle, PrimitiveType::TRIANGLES, 0, 0, 0, 6);
  Mesh mesh {
      .vertex = quad_vertex,
      .index = quad_index,
      .primitive_handle = std::move(handle)
  };
  mesh_cache_[kDefaultQuadKey] = {mesh};
}

uint32_t MeshReader::GetMeshSize() {
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

RenderPrimitiveHandle MeshReader::GetQuadPrimitive() {
  return mesh_cache_.at(kDefaultQuadKey).front().primitive_handle;
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