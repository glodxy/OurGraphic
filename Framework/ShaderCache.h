//
// Created by Glodxy on 2021/10/12.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_SHADERCACHE_H_
#define OUR_GRAPHIC_FRAMEWORK_SHADERCACHE_H_

#include "Backend/include/Program.h"
#include <map>
#include <string>
#include <memory>
#include <vector>

namespace our_graph {
class ShaderCache {
 public:
  ShaderCache();
  ~ShaderCache();

  class ShaderBuilder {
   public:
    ShaderBuilder(std::string shader_name, ShaderCache* cache);
    ShaderBuilder& Vertex(std::string file_path);
    ShaderBuilder& Frag(std::string file_path);
    Program Build();

   private:
    std::vector<uint32_t> CompileFile(Program::ShaderType type, std::string file_path);

    friend class ShaderCache;
    std::string shader_name_;
    std::string vertex_file_path_, frag_file_path_;
    ShaderCache* shader_cache_;
  };
};
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_SHADERCACHE_H_
