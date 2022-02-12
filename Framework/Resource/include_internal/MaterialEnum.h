//
// Created by Glodxy on 2021/12/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_MATERIALENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_MATERIALENUM_H_
#include <bitset>
namespace our_graph {
// 最大可自定义的变量数为4
static constexpr size_t MATERIAL_VARIABLES_COUNT = 4;
using AttributeBitset = std::bitset<32>;
/**
 * Material domains
 * 材质的作用域
 */
enum class MaterialDomain : uint8_t {
  SURFACE         = 0, //!< shaders applied to renderables
  POST_PROCESS    = 1, //!< shaders applied to rendered buffers
};

/**
 * Supported shading models
 */
enum class ShadingModel : uint8_t {
  UNKNOWN =0,
  UNLIT,                  //!< no lighting applied, emissive possible
  LIT,                    //!< default, standard lighting
  SUBSURFACE,             //!< subsurface lighting model
  CLOTH,                  //!< cloth lighting model
  SPECULAR_GLOSSINESS,    //!< legacy lighting model
};
#if WIN32
#undef OPAQUE
#undef TRANSPARENT
#endif
/**
 * copy from filament
 * Supported blending modes
 */
enum class BlendingMode : uint8_t {
  //! material is opaque
  OPAQUE,
  //! material is transparent and color is alpha-pre-multiplied, affects diffuse lighting only
  TRANSPARENT,
  //! material is additive (e.g.: hologram)
  ADD,
  //! material is masked (i.e. alpha tested)
  MASKED,
  /**
   * material is transparent and color is alpha-pre-multiplied, affects specular lighting
   * when adding more entries, change the size of FRenderer::CommandKey::blending
   */
  FADE,
  //! material darkens what's behind it
  MULTIPLY,
  //! material brightens what's behind it
  SCREEN,
};

/**
 * How transparent objects are handled
 */
enum class TransparencyMode : uint8_t {
  //! the transparent object is drawn honoring the raster state
  DEFAULT,
  /**
   * the transparent object is first drawn in the depth buffer,
   * then in the color buffer, honoring the culling mode, but ignoring the depth test function
   */
  TWO_PASSES_ONE_SIDE,

  /**
   * the transparent object is drawn twice in the color buffer,
   * first with back faces only, then with front faces; the culling
   * mode is ignored. Can be combined with two-sided lighting
   */
  TWO_PASSES_TWO_SIDES
};

/**
 * Refraction
 * 折射的计算模式
 */
enum class RefractionMode : uint8_t {
  NONE            = 0, //!< no refraction
  CUBEMAP         = 1, //!< refracted rays go to the ibl cubemap
  SCREEN_SPACE    = 2, //!< refracted rays go to screen space
};

/**
 * Refraction type
 */
enum class RefractionType : uint8_t {
  SOLID           = 0, //!< refraction through solid objects (e.g. a sphere)
  THIN            = 1, //!< refraction through thin objects (e.g. window)
};


/**
 * 说明了材质所带有的固定属性
 * */
namespace MaterialProperty {
enum Property: uint8_t {
  BASE_COLOR, //! vec4 ,基础颜色
  ROUGHNESS, //! float 粗糙度，只在LIT模型中使用
  METALLIC, //! float 金属度
  REFLECTANCE, //! float Reflectance
  EMISSIVE, //! vec4 自发光
};
}
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_MATERIALENUM_H_
