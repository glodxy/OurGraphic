//
// Created by Glodxy on 2021/10/5.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_BACKENDUTILS_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_BACKENDUTILS_H_
#include "include/PixelBufferDescriptor.h"
#include "Framework/Backend/include/DriverEnum.h"

namespace our_graph {
class BackendUtil {
 public:
  static bool Reshape(const PixelBufferDescriptor& src_data,
               PixelBufferDescriptor& reshape_data);

  static size_t GetFormatSize(TextureFormat format);
};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_BACKENDUTILS_H_
