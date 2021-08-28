//
// Created by Glodxy on 2021/8/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_IRENDERENGINE_H_
#define OUR_GRAPHIC_FRAMEWORK_IRENDERENGINE_H_
#include <memory>
namespace our_graph {
/**
 * 渲染引擎基类
 * 目前的管理结构为IRenderEngine->FixedPipeline->RersourceHandle/Executor
 * 未来的结构为IRenderEngine->RenderGraphMgr->IPipeline->ResourceHandle/Executor
 *
 * */
class IRenderEngine {
 public:
  /**
   * 模块初始化
   * */
  virtual void Init() = 0;

  /**
   * 模块销毁
   * */
  virtual void Destroy() = 0;

  /**
   * 开始渲染循环之前的操作
   * */
  virtual void Start() = 0;

  /**
   * 结束渲染循环之后的操作
   * */
  virtual void End() = 0;


  //////////////////////////////////////////
  // 渲染循环相关函数（每帧调用）
  /////////////////////////////////////////
  /**
   * 渲染前的操作，通常用于资源的配置
   * */
  virtual void BeforeRender() = 0;

  /**
   * 每帧调用的主渲染函数，在此实现主要渲染流程
   * */
  virtual void Render() = 0;

  /**
   * 完成一次渲染后的操作
   * */
  virtual void AfterRender() = 0;


  template<class T, typename =
      std::enable_if<std::is_base_of<IRenderEngine, T>::value>>
  static std::shared_ptr<T> GetInstance() {
    static std::shared_ptr<T> engine =
        std::make_shared<T>(token());
    return engine;
  }

 protected:
  explicit IRenderEngine() = default;
  struct token{};
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_IRENDERENGINE_H_
