//
// Created by chaytian on 2021/11/2.
//

#include "SoftDriver.h"
#include "Backend/include_internal/Dispatcher.h"
#include "SoftSwapChain.h"
namespace our_graph {

SoftDriver::SoftDriver(SDL_Window *window) : DriverApi() {
  dispatcher_ = new Dispatcher<SoftDriver>();
}

SoftDriver::~SoftDriver() {
  delete dispatcher_;
}

SwapChainHandle SoftDriver::CreateSwapChainS() {
  return AllocHandle<SoftSwapChain>();
}

void SoftDriver::CreateSwapChainR(SwapChainHandle handle, void *native_window, uint64_t flags) {
  Construct<SoftSwapChain>(handle);
}

}