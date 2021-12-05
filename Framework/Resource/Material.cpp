//
// Created by Glodxy on 2021/12/5.
//
#include "include/Material.h"
#include "ResourceAllocator.h"
#include "Material/SamplerBindingMap.h"
#include "Framework/include/GlobalEnum.h"
namespace our_graph {
struct Material::Details {
  const void* data = nullptr;
  size_t size = 0;
  MaterialParser* parser = nullptr;
  bool default_material = false;
  Driver* driver = nullptr;
};

using BuilderType = Material;
BuilderType::Builder::Builder(Driver* driver) noexcept {
  impl_->driver = driver;
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

Material::Builder &Material::Builder::Data(const void *data, size_t size) {
  impl_->data = data;
  impl_->size = size;
  return *this;
}

Material *Material::Builder::Build() {
  // todo:创建material parser
  MaterialParser* material_parser = nullptr;
  // 与filament不同，只支持glsl，故不检查着色器语言
  impl_->parser = material_parser;
  return ResourceAllocator::Get().CreateMaterial(*this);
}

/*-------------------Material--------------*/
/**
 * 将block中的所有sampler生成sasmpler group
 * @note: 因为每个block并没有限制绑定位置，
 * 所以需手动传入其相应的binding_point
 * */
static void AddSamplerGroup(Program& program, uint8_t binding_point,
                            SamplerBlock& block,
                            SamplerBindingMap const& map) {
  // 获取sampler的数量
  const size_t sampler_cnt = block.GetSize();
  if (sampler_cnt) {
    // 存在sampler的时候，进行绑定
    std::vector<Program::Sampler> samplers(sampler_cnt);
    auto const& list = block.GetSamplerInfoList();
    for (size_t i = 0; i < sampler_cnt; ++i) {
      // 根据sampler block以及sampler的名称来生成uniform名称
      std::string uniform_name = SamplerBlock::GetUniformName(
          block.GetName(), list[i].name);
      uint8_t binding = 0;
      bool suc = map.GetSamplerBinding(binding_point, (uint8_t)i, &binding);
      if (!suc) {
        LOG_ERROR("Material", "Add Sampler Group Failed! sampler[{}] with binding point[{}] "
                              "cannot get global binding!"
                              "block name:{}, sampler name:{}",
                              i, binding_point, block.GetName(),
                              list[i].name);
        assert(suc);
        return;
      }
      // 材质的采样器必须绑定纹理
      const bool strict = (binding_point == BindingPoints::PER_MATERIAL_INSTANCE);
      samplers[i] = {std::move(uniform_name), binding, strict};
    }
    program.SetSamplerGroup(binding_point, samplers.data(), samplers.size());
  }
}


}  // namespace our_graph
