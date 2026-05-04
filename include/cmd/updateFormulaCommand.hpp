#pragma once

#include "command.hpp"

struct UpdateFormulaCommand : Command {
  QString newLatex;
  QString oldLatex;
  int taskId = -1;
  int formulaId = -1;

  UpdateFormulaCommand(int id, int fId, QString latex);
  ChangeType changeType() const override;

  void redo(Assignment& a) override;

  void undo(Assignment& a) override;

  int resultId() const override;
};
