//
// Created by chaytian on 2021/11/10.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTRENDERTARGET_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTRENDERTARGET_H_
#include <vector>
#include "Backend/include_internal/IResource.h"

namespace our_graph {
class SoftRenderTarget : public IRenderTarget {
  using Color = uint32_t;
 public:
  /**
   * 创建默认的render target， size等同窗口size
   * */
  explicit SoftRenderTarget();
  explicit SoftRenderTarget(uint32_t w, uint32_t h);

  /**
   * 获得所有color的字节数
   * */
  uint64_t GetByteSize() const;

  /**
   * 获取像素个数
   * */
  uint32_t GetSize() const;

  /**
   * 获得数据
   * */
  void const* GetData() const;
  void SetPixel(int x, int y, Color color);

  bool IsOffscreen() const {
    return offscreen_;
  }

  /**
   * 深度相关操作
   * */
  float GetDepth(int x, int y);
  void SetDepth(int x, int y, float depth);

  void ClearDepth();
  void ClearColor();
 private:
  inline uint32_t GetIndex(uint32_t x, uint32_t y);

  std::vector<Color> colors_;
  // 深度缓冲区
  std::vector<float> depth_;
  /**
   * 是否是离屏的（是否会直接显示）
   * 对于非离屏的rendertarget，会直接等同于swapchain的纹理。
   * */
  bool offscreen_;
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_RESOURCE_SOFTRENDERTARGET_H_
