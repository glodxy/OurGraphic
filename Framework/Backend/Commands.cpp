//
// Created by Glodxy on 2021/10/23.
//

#include "include_internal/Commands.h"

#include "include/Driver.h"

namespace our_graph {

CommandStream::CommandStream(DriverApi &driver, utils::CircularBuffer &buffer) noexcept
  : driver_(&driver),
    current_buffer_(&buffer),
    dispatcher_(&driver.GetDispatcher()) {

}

void CommandStream::Execute(void *buffer) {
  // 执行从buffer开始的所有command
  driver_->Execute([this, buffer]() {
    DriverApi& driver = *driver_;
    CommandBase* base = static_cast<CommandBase*>(buffer);
    while (base) {
      base = base->Execute(driver);
    }
  });
}

void CommandStream::QueueCommand(std::function<void()> command) {
  new(AllocateCommand(CustomCommand::Align(sizeof(CustomCommand)))) CustomCommand(std::move(command));
}

void CustomCommand::Execute(DriverApi &, CommandBase *base, intptr_t *next) noexcept {
  *next = CustomCommand::Align(sizeof(CustomCommand));
  static_cast<CustomCommand*>(base)->command_();
  static_cast<CustomCommand*>(base)->~CustomCommand();
}

}  // namespace our_graph