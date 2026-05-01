#pragma once

#include "command.hpp"

struct AddFormulaCommand : Command {
  int createdId = -1;
  int afterId = -1;
  Task* t = nullptr;

  AddFormulaCommand(Task* task, int afterId = -1);
  ChangeType changeType() const override;

  void redo(Assignment& a) override;

  void undo(Assignment& a) override;

  int resultId() const override;
};

