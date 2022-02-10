//
// Created by Glodxy on 2022/2/10.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_IMAGEENCODER_H_
#define OUR_GRAPHIC_UTILS_IMAGE_IMAGEENCODER_H_
#include <string>

#include "LinearImage.h"
namespace our_graph::image {
class ImageEncoder {
 public:
  enum class Format {
    PNG
  };

  static bool Encode(std::ostream& out, Format format, const LinearImage& image);

  class BaseEncoder {
   public:
    virtual bool Encode(const LinearImage& image) = 0;
    virtual ~BaseEncoder() = default;
  };
};
}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_IMAGEENCODER_H_
