//
// Created by chaytian on 2021/11/2.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTSWAPCHAIN_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTSWAPCHAIN_H_
#include "Backend/include_internal/IResource.h"
#include "SoftContext.h"
namespace our_graph {
/**
 * 该类用于管理批量texture进行交换，并将其展示到surface上
 * 会根据context的内容进行创建，不需要额外传递参数
 * */
class SoftSwapChain : public ISwapChain {
  // 此处目前只使用两个纹理进行交换
  static constexpr size_t SWAPCHAIN_TEX_SIZE = 2;
 public:
  SoftSwapChain();
  ~SoftSwapChain() override;

  // 提交并将交换链设为下一项
  void CommitAndAcquireNext();
  // 获取当前的纹理
  SDL_Texture*const  GetCurrent() const;

  // 根据context中的数据进行resize
  void Resize();
 protected:
  void Create();
  void Destroy();


  SDL_Texture* textures_[SWAPCHAIN_TEX_SIZE];
  // 当前交换链的位置
  size_t current_idx_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_SOFT_SOFTSWAPCHAIN_H_
