#pragma once

#include "command.hpp"

struct ToggleAnswerCommand : Command {
  int taskId = -1;
  int formulaId = -1;

  ToggleAnswerCommand(int tId, int fId) : taskId{tId}, formulaId{fId} {}
  ChangeType changeType() const override;

  void redo(Assignment& a) override;
  void undo(Assignment& a) override;

  int resultId() const override;
};

