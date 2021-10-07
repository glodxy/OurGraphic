//
// Created by Glodxy on 2021/8/29.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_IRENDERDEVICE_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_IRENDERDEVICE_H_

#include "IRenderInstance.h"
namespace our_graph {
class IRenderDevice {
 public:
  virtual void CreateDevice(const IRenderInstance* instance) = 0;
  virtual void DestroyDevice() = 0;
 protected:

};
}
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_IRENDERDEVICE_H_
