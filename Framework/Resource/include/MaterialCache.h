//
// Created by Glodxy on 2021/12/28.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIALCACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIALCACHE_H_
#include <map>
#include "Framework/Resource/include/Material.h"
#include "Framework/Backend/include/Driver.h"
namespace our_graph {
/**
 * 该cache用于缓存material文件对应的Material
 * */
class MaterialCache {
 public:
  static Material* GetMaterial(const std::string& file, Driver* driver);

 private:
  static std::map<std::string, Material*> cache_;
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_MATERIALCACHE_H_
