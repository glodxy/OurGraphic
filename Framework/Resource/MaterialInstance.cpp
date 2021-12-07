//
// Created by Glodxy on 2021/12/7.
//

#include "include/MaterialInstance.h"
#include "include/Material.h"
namespace our_graph {

template<size_t S>
void MaterialInstance::SetParameterUntyped(const std::string& name,
                                      const void* value) noexcept {
  size_t offset = material_->GetUniformBlock().GetUniformOffset(name, 0);
  if (offset >= 0) {
    // 当offset 大于等于0时，说明是正常的offset
    uniform_buffer_.SetUniformUntyped<S>(offset, value);
  }
}


}