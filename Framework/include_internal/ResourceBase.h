//
// Created by Glodxy on 2021/10/16.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_INCLUDE_INTERNAL_RESOURCEBASE_H_
#define OUR_GRAPHIC_FRAMEWORK_INCLUDE_INTERNAL_RESOURCEBASE_H_

namespace our_graph {
/**
 * 所有resource的基类，
 * 所有resource都需要继承自该类
 * */
class ResourceBase {
 public:
  virtual ~ResourceBase() = default;
  virtual void Destroy() = 0;


};
}  // namespace our_graph

#endif //OUR_GRAPHIC_FRAMEWORK_INCLUDE_INTERNAL_RESOURCEBASE_H_
