#include <cmd/updateFormulaCommand.hpp>

#include <types.hpp>

UpdateFormulaCommand::UpdateFormulaCommand(int id, int fId, QString latex) 
  : taskId{id}, formulaId{fId}, newLatex{latex} {}

ChangeType UpdateFormulaCommand::changeType() const {
  return ChangeType::Content;
}

void UpdateFormulaCommand::redo(Assignment& a) {
  auto t = a.getTask(taskId);
  if (!t) return;

  auto f = t->getFormula(formulaId);
  if (!f) return;

  if (!initialized) {
    oldLatex = f->latex;
    initialized = true;
  }

  f->latex = newLatex;
}

void UpdateFormulaCommand::undo(Assignment& a) {
  auto t = a.getTask(taskId);
  if (t) {
    auto f = t->getFormula(formulaId);
    if (f) {
      f->latex = oldLatex;
    }
  }
}

int UpdateFormulaCommand::resultId() const {
  return taskId;
}

