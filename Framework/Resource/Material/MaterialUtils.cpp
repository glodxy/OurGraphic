//
// Created by Glodxy on 2022/2/9.
//

#include "MaterialUtils.h"

#include <vector>
#include "Backend/include/Program.h"
#include "Resource/Material/SamplerBlock.h"
#include "Resource/Material/SamplerBindingMap.h"
namespace our_graph {

void MaterialUtils::AddSamplerGroup(Program &program,
                                    uint8_t binding_point,
                                    const SamplerBlock &block,
                                    const SamplerBindingMap &map) {
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

}