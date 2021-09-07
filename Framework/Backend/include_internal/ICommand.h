//
// Created by Glodxy on 2021/9/7.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ICOMMAND_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ICOMMAND_H_

namespace our_graph {
class ICommand {
 public:
  virtual void Execute() = 0;
  virtual int GetCommandType() = 0;
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_INCLUDE_INTERNAL_ICOMMAND_H_
