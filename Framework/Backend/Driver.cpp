//
// Created by Glodxy on 2021/10/24.
//
#include "Backend/include/Driver.h"

#include "Backend/include/DriverApi.h"
#include "Backend/Vulkan/VulkanDriver.h"
#include "Backend/Soft/SoftDriver.h"
#include "Backend/include_internal/CommandBufferQueue.h"
#include "include/Driver.h"
#include "Utils/OGLogging.h"
#if WIN32
#include "Backend/Vulkan/VulkanPlatformWindows.h"
#elif __APPLE__
#include "Backend/Vulkan/VulkanPlatformMacos.h"
#endif

#include <thread>
#include <map>
namespace our_graph {
static std::unique_ptr<IPlatform> CreatePlatform(Backend backend) {
#if __APPLE__
  switch (backend) {
    case Backend::VULKAN:
      return std::make_unique<VulkanPlatformMacos>();
    case Backend::SOFT:
      return nullptr;
  }
#elif WIN32
  switch (backend) {
    case Backend::VULKAN:
      return std::make_unique<VulkanPlatformWindows>();
    case Backend::SOFT:
      return nullptr;
  }
#endif
}

static DriverApi* CreateDriverApi(Backend backend, void* context) {
  switch (backend) {
    case Backend::VULKAN:
      return new VulkanDriver();
    case Backend::SOFT: {
      SDL_Window* window = (SDL_Window*)context;
      return new SoftDriver(window);
    }
  }
}



using DriverMap = std::map<Driver*, std::pair<DriverApi*, CommandBufferQueue*>>;

static DriverMap driver_map;
static Driver* current_driver = nullptr;
static CommandBufferQueue* current_command = nullptr;
static DriverApi* current_api = nullptr;
static std::thread driver_thread;

int DriverLoop();

Driver* CreateDriver(Backend backend, void* context) {
  uint32_t command_buffer_size =  16 * 1024; //每个留出1m的空间
  DriverApi* driver_api = CreateDriverApi(backend, context);
  CommandBufferQueue* buffer_queue =
      new CommandBufferQueue(command_buffer_size,
                             COMMAND_BUFFER_MAX_CNT * command_buffer_size);
  Driver* driver = new Driver(driver_api, buffer_queue->GetCircularBuffer());
  std::unique_ptr<IPlatform> platform = CreatePlatform(backend);
  driver->Init(std::move(platform));
  driver_map.insert({driver, {driver_api, buffer_queue}});
  current_driver = driver;
  current_command = buffer_queue;
  current_api = driver_api;

  driver_thread = std::thread(&DriverLoop);
  return driver;
}

void FlushDriverCommand() {
  //todo: 清除临时构造资源
  current_api->Purge();
  current_command->Flush();
}

void DestroyDriver(Driver* driver) {
  LOG_INFO("DriverThread", "DestroyDriver CALL!");
  // 1. 先刷新当前的command queue
  FlushDriverCommand();

  // 2. 请求exit
  current_command->RequestExit();

  // 3. 阻塞driver线程直到完成释放
  driver_thread.join();
}

bool Execute() {
  auto buffers = current_command->WaitForCommands();
  // 此处因为command queue会等待输入后才返回
  // 所以除了exit以外是不会为空的
  if (buffers.empty()) {
    return false;
  }

  // 执行buffers中所有的command
  for (auto& item : buffers) {
    if (item.begin) {
      current_driver->Execute(item.begin);
      current_command->ReleaseBuffer(item);
    }
  }

  return true;
}


int DriverLoop() {
  uint32_t id = std::thread::hardware_concurrency() - 1;

  while (true) {
    if (!Execute()) {
      break;
    }
  }
  LOG_INFO("DriverThread", "Start Clear Driver Res!");
  current_api->Purge();
  current_driver->Clear();
  auto iter = driver_map.find(current_driver);
  if (iter == driver_map.end()) {
    LOG_ERROR("DriverBuider", "Destroy Failed! Not in map, map size:{}",
              driver_map.size());
  }
  delete iter->first;
  delete iter->second.first;
  delete iter->second.second;
  driver_map.erase(iter);
  current_command = nullptr;
  current_driver = nullptr;
  current_api = nullptr;

  return 0;
}


}  // namespace our_graph