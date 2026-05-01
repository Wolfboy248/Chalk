#pragma once

#include "command.hpp"

struct UpdateAssignmentTitleCommand : Command {
  QString oldTitle;
  QString newTitle;

  UpdateAssignmentTitleCommand(QString newTitle);
  ChangeType changeType() const override;

  void redo(Assignment& a) override;

  void undo(Assignment& a) override;
};
