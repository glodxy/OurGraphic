//
// Created by Glodxy on 2021/12/5.
//
/**
 * 该文件用于定义全局的enum以及config常量
 *
 * */
#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_GLOBALENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_GLOBALENUM_H_
#include "Backend/include/DriverEnum.h"

namespace our_graph {

namespace {
/**
 * 该enum用于区分资源在shader中的使用
 * uniform buffer以及sampler的绑定都会借助该enum来找到对应的location 或 binding
 * */
enum BindingPoints :uint8_t {
  PER_VIEW = 0, // 每次视图更新所使用的uniform/sampler插槽
  PER_RENDERABLE = 1, // 每个物体更新所使用的uniform/sampler
  PER_MATERIAL_INSTANCE, // 每个材质的uniform/sampler
  LIGHT, // 用于光照的uniform/sampler
  COUNT
};
}

// 最大的光照数
constexpr size_t CONFIG_MAX_LIGHT_COUNT = 256;
constexpr size_t CONFIG_MAX_LIGHT_IDX = CONFIG_MAX_LIGHT_COUNT - 1;

// render graph相关的设置
// render graph中的resource所cache的容量
static constexpr size_t RG_RESOURCE_CACHE_CAPACITY = 64u << 20u;   // 64 MiB
// render graph中资源的最大存在周期
static constexpr size_t RG_RESOURCE_CACHE_MAX_AGE  = 30u;

namespace ShaderVariantBit {
  static constexpr uint32_t DEFERRED_LIGHT         = 0x01; // 是否使用延迟光照模块
  static constexpr uint32_t DIRECTIONAL_LIGHTING   = 0x02; // directional light present, per frame/world position
  static constexpr uint32_t DYNAMIC_LIGHTING       = 0x04; // 是否启用动态光照

  static constexpr uint32_t MAX                    = 0xffffffff;
  static constexpr uint32_t MAX_BIT                = 32;
}
// 限制前端部分使用的插槽不超过后端部分定义的插槽
static_assert(BindingPoints::COUNT <= CONFIG_BINDING_COUNT);

namespace {
//! gbuffer的纹理数
static constexpr size_t GBUFFER_MAX_SIZE = 5;
// 延迟光照的sampler绑定
// todo:目前都是rgba8格式
enum DeferredLightInputBinding : uint8_t {
  GBUFFER_A = 0,
  GBUFFER_B,
  GBUFFER_C,
  GBUFFER_D,
  GBUFFER_E,
  MAX
};
static_assert(DeferredLightInputBinding::MAX <= GBUFFER_MAX_SIZE);
}

static constexpr size_t GLOBAL_SHADER_FILE_COUNT = 2;
//! 该变量控制了存在的global shader的单个文件
enum GlobalShaderFileType : uint8_t {
  TEXTUREQUAD_VS = 0, // 用于全屏texture的vs（light pass的vs），仅用于得到一个纹理
  DEFERRED_LIGHT_PASS_FS  = 1, // 使用gbuffer的light pass
};
// global shader对应的file
static const char* GLOBAL_SHADER_FILES[] = {
    "Shared/texturequad.vs",
    "Shared/deferred_light.fs",
};
//! 一组shader的类型，比如deferred light = texturequad_vs + deferred_light_pass_fs
enum GlobalShaderType : uint8_t {
  DEFERRED_LIGHT = 0
};
static_assert(sizeof(GLOBAL_SHADER_FILES)/sizeof(const char*) == GLOBAL_SHADER_FILE_COUNT);

}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_GLOBALENUM_H_
