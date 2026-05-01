#pragma once

#include "command.hpp"

struct RemoveTaskCommand : Command {
  int taskId;
  std::unique_ptr<Task> backup;
  int index;

  RemoveTaskCommand(int id) : taskId{id} {}
  ChangeType changeType() const override;

  void redo(Assignment& a) override;
  void undo(Assignment& a) override;

  int resultId() const override;
};

