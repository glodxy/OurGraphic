//
// Created by Glodxy on 2021/12/5.
//

#include "UniformBlock.h"

namespace our_graph {
/*-------------Builder---------------*/
UniformBlock::Builder & UniformBlock::Builder::Name(const std::string &name) {
  name_ = name;
  return *this;
}

UniformBlock::Builder & UniformBlock::Builder::Add(const std::string &name, size_t size, Type type) {
  entries_.emplace_back(name, size, type);
  return *this;
}


/*----------------------------------*/
}  // namespace our_graph