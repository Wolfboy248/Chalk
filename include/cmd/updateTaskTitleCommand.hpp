#pragma once

#include "command.hpp"

struct UpdateTaskTitleCommand : Command {
  QString oldTitle;
  QString newTitle;
  int taskId = -1;

  UpdateTaskTitleCommand(int id, QString newTitle);
  ChangeType changeType() const override;

  void redo(Assignment& a) override;

  void undo(Assignment& a) override;

  int resultId() const override;
};
