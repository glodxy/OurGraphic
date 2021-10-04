//
// Created by Glodxy on 2021/9/24.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_

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
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
