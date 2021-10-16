//
// Created by Glodxy on 2021/9/24.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
#include <array>
#include <cstdint>
#include <cstddef>
#include "glm/glm.hpp"
#include "Utils/BitmaskEnum.h"
namespace our_graph {
static constexpr uint8_t MIN_SUPPORTED_RENDER_TARGET_COUNT = 4u;
static constexpr uint8_t MAX_SUPPORTED_RENDER_TARGET_COUNT = 8u;


static constexpr size_t CONFIG_BINDING_COUNT = 8;

// 最大采样数量
static constexpr size_t MAX_SAMPLER_COUNT = 16;
// 最大可使用的顶点数量
static constexpr size_t MAX_VERTEX_ATTRIBUTE_COUNT = 16;
// 最大的顶点缓冲数
static constexpr size_t MAX_VERTEX_BUFFER_COUNT = 16;
// 最大可使用的command buffer数
static constexpr uint32_t MAX_COMMAND_BUFFERS_COUNT = 10;
// pipeline最长等待周期，超过10个周期未使用则销毁
static constexpr uint32_t MAX_PIPELINE_AGE = 10;

static constexpr uint32_t VMA_BUFFER_POOL_BLOCK_SIZE_IN_MB = 128;
/**
 * Bitmask for selecting render buffers
 */
enum class TargetBufferFlags : uint32_t {
  NONE = 0x0u,                            //!< No buffer selected.
  COLOR0 = 0x00000001u,                   //!< Color buffer selected.
  COLOR1 = 0x00000002u,                   //!< Color buffer selected.
  COLOR2 = 0x00000004u,                   //!< Color buffer selected.
  COLOR3 = 0x00000008u,                   //!< Color buffer selected.
  COLOR4 = 0x00000010u,                   //!< Color buffer selected.
  COLOR5 = 0x00000020u,                   //!< Color buffer selected.
  COLOR6 = 0x00000040u,                   //!< Color buffer selected.
  COLOR7 = 0x00000080u,                   //!< Color buffer selected.

  COLOR = COLOR0,                         //!< \deprecated
  COLOR_ALL = COLOR0 | COLOR1 | COLOR2 | COLOR3 | COLOR4 | COLOR5 | COLOR6 | COLOR7,
  DEPTH   = 0x10000000u,                  //!< Depth buffer selected.
  STENCIL = 0x20000000u,                  //!< Stencil buffer selected.
  DEPTH_AND_STENCIL = DEPTH | STENCIL,    //!< depth and stencil buffer selected.
  ALL = COLOR_ALL | DEPTH | STENCIL       //!< Color, depth and stencil buffer selected.
};

/**
 * 像素的数据格式
 * */
enum class PixelDataFormat : uint8_t {
  R,                  //!< One Red channel, float
  R_INTEGER,          //!< One Red channel, integer
  RG,                 //!< Two Red and Green channels, float
  RG_INTEGER,         //!< Two Red and Green channels, integer
  RGB,                //!< Three Red, Green and Blue channels, float
  RGB_INTEGER,        //!< Three Red, Green and Blue channels, integer
  RGBA,               //!< Four Red, Green, Blue and Alpha channels, float
  RGBA_INTEGER,       //!< Four Red, Green, Blue and Alpha channels, integer
  UNUSED,             // used to be rgbm
  DEPTH_COMPONENT,    //!< Depth, 16-bit or 24-bits usually
  DEPTH_STENCIL,      //!< Two Depth (24-bits) + Stencil (8-bits) channels
  ALPHA               //! One Alpha channel, float
};

/**
 * 像素数据的类型
 * */
enum class PixelDataType : uint8_t {
  UBYTE,                //!< unsigned byte
  BYTE,                 //!< signed byte
  USHORT,               //!< unsigned short (16-bit)
  SHORT,                //!< signed short (16-bit)
  UINT,                 //!< unsigned int (16-bit)
  INT,                  //!< signed int (32-bit)
  HALF,                 //!< half-float (16-bit float)
  FLOAT,                //!< float (32-bits float)
  COMPRESSED,           //!< compressed pixels, @see CompressedPixelDataType
  UINT_10F_11F_11F_REV, //!< three low precision floating-point numbers
  USHORT_565,           //!< unsigned int (16-bit), encodes 3 RGB channels
  UINT_2_10_10_10_REV,  //!< unsigned normalized 10 bits RGB, 2 bits alpha
};

/**
 * Supported element types
 */
enum class ElementType : uint8_t {
  BYTE,
  BYTE2,
  BYTE3,
  BYTE4,
  UBYTE,
  UBYTE2,
  UBYTE3,
  UBYTE4,
  SHORT,
  SHORT2,
  SHORT3,
  SHORT4,
  USHORT,
  USHORT2,
  USHORT3,
  USHORT4,
  INT,
  UINT,
  FLOAT,
  FLOAT2,
  FLOAT3,
  FLOAT4,
  HALF,
  HALF2,
  HALF3,
  HALF4,
};

/**
 * Vertex attribute types
 */
enum VertexAttribute : uint8_t {
  // Update hasIntegerTarget() in VertexBuffer when adding an attribute that will
  // be read as integers in the shaders

  POSITION        = 0, //!< XYZ position (float3)
  TANGENTS        = 1, //!< tangent, bitangent and normal, encoded as a quaternion (float4)
  COLOR           = 2, //!< vertex color (float4)
  UV0             = 3, //!< texture coordinates (float2)
  UV1             = 4, //!< texture coordinates (float2)
  BONE_INDICES    = 5, //!< indices of 4 bones, as unsigned integers (uvec4)
  BONE_WEIGHTS    = 6, //!< weights of the 4 bones (normalized float4)
  // -- we have 1 unused slot here --
  CUSTOM0         = 8,
  CUSTOM1         = 9,
  CUSTOM2         = 10,
  CUSTOM3         = 11,
  CUSTOM4         = 12,
  CUSTOM5         = 13,
  CUSTOM6         = 14,
  CUSTOM7         = 15,

  // Aliases for vertex morphing.
  MORPH_POSITION_0 = CUSTOM0,
  MORPH_POSITION_1 = CUSTOM1,
  MORPH_POSITION_2 = CUSTOM2,
  MORPH_POSITION_3 = CUSTOM3,
  MORPH_TANGENTS_0 = CUSTOM4,
  MORPH_TANGENTS_1 = CUSTOM5,
  MORPH_TANGENTS_2 = CUSTOM6,
  MORPH_TANGENTS_3 = CUSTOM7,

  // this is limited by driver::MAX_VERTEX_ATTRIBUTE_COUNT
};

//! Buffer object binding type
enum class BufferObjectBinding : uint8_t {
  VERTEX,
  UNIFORM
};

//! Face culling Mode
enum class CullingMode : uint8_t {
  NONE,               //!< No culling, front and back faces are visible
  FRONT,              //!< Front face culling, only back faces are visible
  BACK,               //!< Back face culling, only front faces are visible
  FRONT_AND_BACK      //!< Front and Back, geometry is not visible
};

//! Vertex attribute descriptor
struct Attribute {
  //! attribute is normalized (remapped between 0 and 1)
  static constexpr uint8_t FLAG_NORMALIZED     = 0x1;
  //! attribute is an integer
  static constexpr uint8_t FLAG_INTEGER_TARGET = 0x2;
  static constexpr uint8_t BUFFER_UNUSED = 0xFF;
  uint32_t offset = 0;                    //!< attribute offset in bytes
  uint8_t stride = 0;                     //!< attribute stride in bytes
  uint8_t buffer = BUFFER_UNUSED;         //!< attribute buffer index
  ElementType type = ElementType::BYTE;   //!< attribute element type
  uint8_t flags = 0x0;                    //!< attribute flags
};

using AttributeArray = std::array<Attribute, MAX_VERTEX_ATTRIBUTE_COUNT>;


/**
 * Primitive types
 */
enum class PrimitiveType : uint8_t {
  // don't change the enums values (made to match GL)
  POINTS      = 0,    //!< points
  LINES       = 1,    //!< lines
  TRIANGLES   = 4,    //!< triangles
  NONE        = 0xFF
};


enum class TextureFormat : uint16_t {
  // 8-bits per element
  R8, R8_SNORM, R8UI, R8I, STENCIL8,

  // 16-bits per element
  R16F, R16UI, R16I,
  RG8, RG8_SNORM, RG8UI, RG8I,
  RGB565,
  RGB9_E5, // 9995 is actually 32 bpp but it's here for historical reasons.
  RGB5_A1,
  RGBA4,
  DEPTH16,

  // 24-bits per element
  RGB8, SRGB8, RGB8_SNORM, RGB8UI, RGB8I,
  DEPTH24,

  // 32-bits per element
  R32F, R32UI, R32I,
  RG16F, RG16UI, RG16I,
  R11F_G11F_B10F,
  RGBA8, SRGB8_A8,RGBA8_SNORM,
  UNUSED, // used to be rgbm
  RGB10_A2, RGBA8UI, RGBA8I,
  DEPTH32F, DEPTH24_STENCIL8, DEPTH32F_STENCIL8,

  // 48-bits per element
  RGB16F, RGB16UI, RGB16I,

  // 64-bits per element
  RG32F, RG32UI, RG32I,
  RGBA16F, RGBA16UI, RGBA16I,

  // 96-bits per element
  RGB32F, RGB32UI, RGB32I,

  // 128-bits per element
  RGBA32F, RGBA32UI, RGBA32I,

  // compressed formats

  // Mandatory in GLES 3.0 and GL 4.3
  EAC_R11, EAC_R11_SIGNED, EAC_RG11, EAC_RG11_SIGNED,
  ETC2_RGB8, ETC2_SRGB8,
  ETC2_RGB8_A1, ETC2_SRGB8_A1,
  ETC2_EAC_RGBA8, ETC2_EAC_SRGBA8,

  // Available everywhere except Android/iOS
  DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA,
  DXT1_SRGB, DXT1_SRGBA, DXT3_SRGBA, DXT5_SRGBA,

  // ASTC formats are available with a GLES extension
  RGBA_ASTC_4x4,
  RGBA_ASTC_5x4,
  RGBA_ASTC_5x5,
  RGBA_ASTC_6x5,
  RGBA_ASTC_6x6,
  RGBA_ASTC_8x5,
  RGBA_ASTC_8x6,
  RGBA_ASTC_8x8,
  RGBA_ASTC_10x5,
  RGBA_ASTC_10x6,
  RGBA_ASTC_10x8,
  RGBA_ASTC_10x10,
  RGBA_ASTC_12x10,
  RGBA_ASTC_12x12,
  SRGB8_ALPHA8_ASTC_4x4,
  SRGB8_ALPHA8_ASTC_5x4,
  SRGB8_ALPHA8_ASTC_5x5,
  SRGB8_ALPHA8_ASTC_6x5,
  SRGB8_ALPHA8_ASTC_6x6,
  SRGB8_ALPHA8_ASTC_8x5,
  SRGB8_ALPHA8_ASTC_8x6,
  SRGB8_ALPHA8_ASTC_8x8,
  SRGB8_ALPHA8_ASTC_10x5,
  SRGB8_ALPHA8_ASTC_10x6,
  SRGB8_ALPHA8_ASTC_10x8,
  SRGB8_ALPHA8_ASTC_10x10,
  SRGB8_ALPHA8_ASTC_12x10,
  SRGB8_ALPHA8_ASTC_12x12,
};

//! Bitmask describing the intended Texture Usage
enum class TextureUsage : uint8_t {
  NONE                = 0x0,
  COLOR_ATTACHMENT    = 0x1,                      //!< Texture can be used as a color attachment
  DEPTH_ATTACHMENT    = 0x2,                      //!< Texture can be used as a depth attachment
  STENCIL_ATTACHMENT  = 0x4,                      //!< Texture can be used as a stencil attachment
  UPLOADABLE          = 0x8,                      //!< Data can be uploaded into this texture (default)
  SAMPLEABLE          = 0x10,                     //!< Texture can be sampled (default)
  SUBPASS_INPUT       = 0x20,                     //!< Texture can be used as a subpass input
  DEFAULT             = UPLOADABLE | SAMPLEABLE   //!< Default texture usage
};


//! Compressed pixel data types
enum class CompressedPixelDataType : uint16_t {
  // Mandatory in GLES 3.0 and GL 4.3
  EAC_R11, EAC_R11_SIGNED, EAC_RG11, EAC_RG11_SIGNED,
  ETC2_RGB8, ETC2_SRGB8,
  ETC2_RGB8_A1, ETC2_SRGB8_A1,
  ETC2_EAC_RGBA8, ETC2_EAC_SRGBA8,

  // Available everywhere except Android/iOS
  DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA,
  DXT1_SRGB, DXT1_SRGBA, DXT3_SRGBA, DXT5_SRGBA,

  // ASTC formats are available with a GLES extension
  RGBA_ASTC_4x4,
  RGBA_ASTC_5x4,
  RGBA_ASTC_5x5,
  RGBA_ASTC_6x5,
  RGBA_ASTC_6x6,
  RGBA_ASTC_8x5,
  RGBA_ASTC_8x6,
  RGBA_ASTC_8x8,
  RGBA_ASTC_10x5,
  RGBA_ASTC_10x6,
  RGBA_ASTC_10x8,
  RGBA_ASTC_10x10,
  RGBA_ASTC_12x10,
  RGBA_ASTC_12x12,
  SRGB8_ALPHA8_ASTC_4x4,
  SRGB8_ALPHA8_ASTC_5x4,
  SRGB8_ALPHA8_ASTC_5x5,
  SRGB8_ALPHA8_ASTC_6x5,
  SRGB8_ALPHA8_ASTC_6x6,
  SRGB8_ALPHA8_ASTC_8x5,
  SRGB8_ALPHA8_ASTC_8x6,
  SRGB8_ALPHA8_ASTC_8x8,
  SRGB8_ALPHA8_ASTC_10x5,
  SRGB8_ALPHA8_ASTC_10x6,
  SRGB8_ALPHA8_ASTC_10x8,
  SRGB8_ALPHA8_ASTC_10x10,
  SRGB8_ALPHA8_ASTC_12x10,
  SRGB8_ALPHA8_ASTC_12x12,
};

//! Face offsets for all faces of a cubemap
struct FaceOffsets {
  using size_type = size_t;
  union {
    struct {
      size_type px;   //!< +x face offset in bytes
      size_type nx;   //!< -x face offset in bytes
      size_type py;   //!< +y face offset in bytes
      size_type ny;   //!< -y face offset in bytes
      size_type pz;   //!< +z face offset in bytes
      size_type nz;   //!< -z face offset in bytes
    };
    size_type offsets[6];
  };
  size_type  operator[](size_t n) const noexcept { return offsets[n]; }
  size_type& operator[](size_t n) { return offsets[n]; }
  FaceOffsets() noexcept = default;
  explicit FaceOffsets(size_type faceSize) noexcept {
    px = faceSize * 0;
    nx = faceSize * 1;
    py = faceSize * 2;
    ny = faceSize * 3;
    pz = faceSize * 4;
    nz = faceSize * 5;
  }
  FaceOffsets(const FaceOffsets& rhs) noexcept {
    px = rhs.px;
    nx = rhs.nx;
    py = rhs.py;
    ny = rhs.ny;
    pz = rhs.pz;
    nz = rhs.nz;
  }
  FaceOffsets& operator=(const FaceOffsets& rhs) noexcept {
    px = rhs.px;
    nx = rhs.nx;
    py = rhs.py;
    ny = rhs.ny;
    pz = rhs.pz;
    nz = rhs.nz;
    return *this;
  }
};

//! blending equation function
enum class BlendEquation : uint8_t {
  ADD,                    //!< the fragment is added to the color buffer
  SUBTRACT,               //!< the fragment is subtracted from the color buffer
  REVERSE_SUBTRACT,       //!< the color buffer is subtracted from the fragment
  MIN,                    //!< the min between the fragment and color buffer
  MAX                     //!< the max between the fragment and color buffer
};

//! blending function
enum class BlendFunction : uint8_t {
  ZERO,                   //!< f(src, dst) = 0
  ONE,                    //!< f(src, dst) = 1
  SRC_COLOR,              //!< f(src, dst) = src
  ONE_MINUS_SRC_COLOR,    //!< f(src, dst) = 1-src
  DST_COLOR,              //!< f(src, dst) = dst
  ONE_MINUS_DST_COLOR,    //!< f(src, dst) = 1-dst
  SRC_ALPHA,              //!< f(src, dst) = src.a
  ONE_MINUS_SRC_ALPHA,    //!< f(src, dst) = 1-src.a
  DST_ALPHA,              //!< f(src, dst) = dst.a
  ONE_MINUS_DST_ALPHA,    //!< f(src, dst) = 1-dst.a
  SRC_ALPHA_SATURATE      //!< f(src, dst) = (1,1,1) * min(src.a, 1 - dst.a), 1
};
//////////////////Sampler////////////////////////

//! Texture sampler type
enum class SamplerType : uint8_t {
  SAMPLER_2D,         //!< 2D texture
  SAMPLER_2D_ARRAY,   //!< 2D array texture
  SAMPLER_CUBEMAP,    //!< Cube map texture
  SAMPLER_EXTERNAL,   //!< External texture
  SAMPLER_3D,         //!< 3D texture
};

//! Sampler Wrap mode
enum class SamplerWrapMode : uint8_t {
  CLAMP_TO_EDGE,      //!< clamp-to-edge. The edge of the texture extends to infinity.
  REPEAT,             //!< repeat. The texture infinitely repeats in the wrap direction.
  MIRRORED_REPEAT,    //!< mirrored-repeat. The texture infinitely repeats and mirrors in the wrap direction.
};

//! Sampler minification filter
enum class SamplerMinFilter : uint8_t {
  // don't change the enums values
  NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
  LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
  NEAREST_MIPMAP_NEAREST = 2, //!< Mip-mapping is activated. But no filtering occurs.
  LINEAR_MIPMAP_NEAREST = 3,  //!< Box filtering within a mip-map level.
  NEAREST_MIPMAP_LINEAR = 4,  //!< Mip-map levels are interpolated, but no other filtering occurs.
  LINEAR_MIPMAP_LINEAR = 5    //!< Both interpolated Mip-mapping and linear filtering are used.
};

//! Sampler magnification filter
enum class SamplerMagFilter : uint8_t {
  // don't change the enums values
  NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
  LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
};

//! Sampler compare mode
enum class SamplerCompareMode : uint8_t {
  // don't change the enums values
  NONE = 0,
  COMPARE_TO_TEXTURE = 1
};

//! comparison function for the depth sampler
enum class SamplerCompareFunc : uint8_t {
  // don't change the enums values
  LE = 0,     //!< Less or equal
  GE,         //!< Greater or equal
  L,          //!< Strictly less than
  G,          //!< Strictly greater than
  E,          //!< Equal
  NE,         //!< Not equal
  A,          //!< Always. Depth testing is deactivated.
  N           //!< Never. The depth test always fails.
};

//! Sampler paramters
struct SamplerParams { // NOLINT
  union {
    struct {
      SamplerMagFilter filterMag      : 1;    //!< magnification filter (NEAREST)
      SamplerMinFilter filterMin      : 3;    //!< minification filter  (NEAREST)
      SamplerWrapMode wrapS           : 2;    //!< s-coordinate wrap mode (CLAMP_TO_EDGE)
      SamplerWrapMode wrapT           : 2;    //!< t-coordinate wrap mode (CLAMP_TO_EDGE)

      SamplerWrapMode wrapR           : 2;    //!< r-coordinate wrap mode (CLAMP_TO_EDGE)
      uint8_t anisotropyLog2          : 3;    //!< anisotropy level (0)
      SamplerCompareMode compareMode  : 1;    //!< sampler compare mode (NONE)
      uint8_t padding0                : 2;    //!< reserved. must be 0.

      SamplerCompareFunc compareFunc  : 3;    //!< sampler comparison function (LE)
      uint8_t padding1                : 5;    //!< reserved. must be 0.

      uint8_t padding2                : 8;    //!< reserved. must be 0.
    };
    uint32_t u;
  };
 private:
  friend inline bool operator < (SamplerParams lhs, SamplerParams rhs) {
    return lhs.u < rhs.u;
  }
};

////////////////////////Raster/////////////////
//! Raster state descriptor
struct RasterState {

  using CullingMode = CullingMode;
  using DepthFunc = SamplerCompareFunc;
  using BlendEquation = BlendEquation;
  using BlendFunction = BlendFunction;

  RasterState() noexcept { // NOLINT
    static_assert(sizeof(RasterState) == sizeof(uint32_t),
                  "RasterState size not what was intended");
    culling = CullingMode::BACK;
    blendEquationRGB = BlendEquation::ADD;
    blendEquationAlpha = BlendEquation::ADD;
    blendFunctionSrcRGB = BlendFunction::ONE;
    blendFunctionSrcAlpha = BlendFunction::ONE;
    blendFunctionDstRGB = BlendFunction::ZERO;
    blendFunctionDstAlpha = BlendFunction::ZERO;
  }

  bool operator == (RasterState rhs) const noexcept { return u == rhs.u; }
  bool operator != (RasterState rhs) const noexcept { return u != rhs.u; }

  void disableBlending() noexcept {
    blendEquationRGB = BlendEquation::ADD;
    blendEquationAlpha = BlendEquation::ADD;
    blendFunctionSrcRGB = BlendFunction::ONE;
    blendFunctionSrcAlpha = BlendFunction::ONE;
    blendFunctionDstRGB = BlendFunction::ZERO;
    blendFunctionDstAlpha = BlendFunction::ZERO;
  }

  // note: clang reduces this entire function to a simple load/mask/compare
  bool hasBlending() const noexcept {
    // This is used to decide if blending needs to be enabled in the h/w
    return !(blendEquationRGB == BlendEquation::ADD &&
        blendEquationAlpha == BlendEquation::ADD &&
        blendFunctionSrcRGB == BlendFunction::ONE &&
        blendFunctionSrcAlpha == BlendFunction::ONE &&
        blendFunctionDstRGB == BlendFunction::ZERO &&
        blendFunctionDstAlpha == BlendFunction::ZERO);
  }

  union {
    struct {
      //! culling mode
      CullingMode culling                 : 2;        //  2

      //! blend equation for the red, green and blue components
      BlendEquation blendEquationRGB      : 3;        //  5
      //! blend equation for the alpha component
      BlendEquation blendEquationAlpha    : 3;        //  8

      //! blending function for the source color
      BlendFunction blendFunctionSrcRGB   : 4;        // 12
      //! blending function for the source alpha
      BlendFunction blendFunctionSrcAlpha : 4;        // 16
      //! blending function for the destination color
      BlendFunction blendFunctionDstRGB   : 4;        // 20
      //! blending function for the destination alpha
      BlendFunction blendFunctionDstAlpha : 4;        // 24

      //! Whether depth-buffer writes are enabled
      bool depthWrite                     : 1;        // 25
      //! Depth test function
      DepthFunc depthFunc                 : 3;        // 28

      //! Whether color-buffer writes are enabled
      bool colorWrite                     : 1;        // 29

      //! use alpha-channel as coverage mask for anti-aliasing
      bool alphaToCoverage                : 1;        // 30

      //! whether front face winding direction must be inverted
      bool inverseFrontFaces              : 1;        // 31

      //! padding, must be 0
      uint8_t padding                     : 1;        // 32
    };
    uint32_t u = 0;
  };
};

/**
 * Selects which buffers to clear at the beginning of the render pass, as well as which buffers
 * can be discarded at the beginning and end of the render pass.
 *
 */
struct RenderPassFlags {
  /**
   * bitmask indicating which buffers to clear at the beginning of a render pass.
   * This implies discard.
   */
  TargetBufferFlags clear;

  /**
   * bitmask indicating which buffers to discard at the beginning of a render pass.
   * Discarded buffers have uninitialized content, they must be entirely drawn over or cleared.
   */
  TargetBufferFlags discardStart;

  /**
   * bitmask indicating which buffers to discard at the end of a render pass.
   * Discarded buffers' content becomes invalid, they must not be read from again.
   */
  TargetBufferFlags discardEnd;
};

/**
 * Frequency at which a buffer is expected to be modified and used. This is used as an hint
 * for the driver to make better decisions about managing memory internally.
 */
enum class BufferUsage : uint8_t {
  STATIC,      //!< content modified once, used many times
  DYNAMIC,     //!< content modified frequently, used many times
  STREAM,      //!< content invalidated and modified frequently, used many times
};

/**
 * Defines a viewport, which is the origin and extent of the clip-space.
 * All drawing is clipped to the viewport.
 */
struct Viewport {
  int32_t left;       //!< left coordinate in window space.
  int32_t bottom;     //!< bottom coordinate in window space.
  uint32_t width;     //!< width in pixels
  uint32_t height;    //!< height in pixels
  //! get the right coordinate in window space of the viewport
  int32_t right() const noexcept { return left + width; }
  //! get the top coordinate in window space of the viewport
  int32_t top() const noexcept { return bottom + height; }
};

/**
 * Specifies the mapping of the near_ and far_ clipping plane to window coordinates.
 */
struct DepthRange {
  float near_ = 0.0f;    //!< mapping of the near_ plane to window coordinates.
  float far_ = 1.0f;     //!< mapping of the far_ plane to window coordinates.
};

/**
 * Parameters of a render pass.
 */
struct RenderPassParams {
  RenderPassFlags flags{};    //!< operations performed on the buffers for this pass

  Viewport viewport{};        //!< viewport for this pass
  DepthRange depthRange{};    //!< depth range for this pass

  //! Color to use to clear the COLOR buffer. RenderPassFlags::clear must be set.
  glm::vec4 clearColor = {};

  //! Depth value to clear the depth buffer with
  double clearDepth = 0.0;

  //! Stencil value to clear the stencil buffer with
  uint32_t clearStencil = 0;

  /**
   * The subpass mask specifies which color attachments are designated for read-back in the second
   * subpass. If this is zero, the render pass has only one subpass. The least significant bit
   * specifies that the first color attachment in the render target is a subpass input.
   *
   * For now only 2 subpasses are supported, so only the lower 4 bits are used, one for each color
   * attachment (see MRT::TARGET_COUNT).
   */
  uint32_t subpassMask = 0;
};


struct PolygonOffset {
  float slope = 0;        // factor in GL-speak
  float constant = 0;     // units in GL-speak
};





template<> struct utils::EnableBitMaskOperators<TargetBufferFlags>
    : public std::true_type {};
template<> struct utils::EnableBitMaskOperators<TextureUsage>
    : public std::true_type {};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
