//
// Created by Glodxy on 2021/9/24.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
#include <array>
#include <cstdint>
#include <cstddef>

namespace our_graph {
static constexpr uint8_t MIN_SUPPORTED_RENDER_TARGET_COUNT = 4u;
static constexpr uint8_t MAX_SUPPORTED_RENDER_TARGET_COUNT = 8u;


static constexpr size_t CONFIG_BINDING_COUNT = 8;

// 最大采样数量
static constexpr size_t MAX_SAMPLER_COUNT = 16;
// 最大可使用的顶点数量
static constexpr size_t MAX_VERTEX_ATTRIBUTE_COUNT = 16;
// 最大可使用的command buffer数
static constexpr uint32_t MAX_COMMAND_BUFFERS_COUNT = 10;
// pipeline最长等待周期，超过10个周期未使用则销毁
static constexpr uint32_t MAX_PIPELINE_AGE = 10;
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

//! Texture sampler type
enum class SamplerType : uint8_t {
  SAMPLER_2D,         //!< 2D texture
  SAMPLER_2D_ARRAY,   //!< 2D array texture
  SAMPLER_CUBEMAP,    //!< Cube map texture
  SAMPLER_EXTERNAL,   //!< External texture
  SAMPLER_3D,         //!< 3D texture
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

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
