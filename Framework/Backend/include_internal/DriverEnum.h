//
// Created by Glodxy on 2021/9/24.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_


namespace our_graph {
static constexpr uint8_t MIN_SUPPORTED_RENDER_TARGET_COUNT = 4u;
static constexpr uint8_t MAX_SUPPORTED_RENDER_TARGET_COUNT = 8u;



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
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_DRIVERENUM_H_
