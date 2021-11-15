//
// Created by chaytian on 2021/11/15.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SIMPLESOFTTEST_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SIMPLESOFTTEST_H_
#include <vector>
#include "Base/TestBase.h"
#include "SoftPipeline.h"
namespace our_graph {
class SimpleSoftTest : public TestBase {
 public:
  void Test(const Pixel *src_pixel, size_t src_size, Pixel *&dst_pixel, size_t &dst_size) override;

 private:
  // 进行深度测试
  void DepthTest(std::vector<Pixel>& pixels);

};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SIMPLESOFTTEST_H_
