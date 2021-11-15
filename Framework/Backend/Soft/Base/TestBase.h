//
// Created by chaytian on 2021/11/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_TESTBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_TESTBASE_H_
#include <cstddef>
#include "ModuleBase.h"
namespace our_graph {
struct Pixel;
class TestBase : public ModuleBase {
 public:
  virtual void Test(const Pixel *src_pixel, size_t src_size, Pixel *&dst_pixel, size_t &dst_size) = 0;

};

}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_BASE_TESTBASE_H_
