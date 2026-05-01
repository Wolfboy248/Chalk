#pragma once

#include "command.hpp"

struct RemoveFormulaCommand : Command {
  int taskId;
  int formulaId;
  std::unique_ptr<Formula> backup;
  int index;

  RemoveFormulaCommand(int tId, int id) : taskId{tId}, formulaId{id} {}
  ChangeType changeType() const override;

  void redo(Assignment& a) override;
  void undo(Assignment& a) override;

  int resultId() const override;
};

