#pragma once

#include "command.hpp"

struct AddTaskCommand : Command {
  QString title;
  int createdId = -1;
  std::unique_ptr<Task> backup;

  AddTaskCommand(QString t) : title{std::move(t)} {}
  ChangeType changeType() const override;

  void redo(Assignment& a) override;

  void undo(Assignment& a) override;

  int resultId() const override;
};
