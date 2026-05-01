#include <cmd/addFormulaCommand.hpp>

#include <types.hpp>

AddFormulaCommand::AddFormulaCommand(Task* task, int aId)
  : t{task}, afterId{aId} {}

ChangeType AddFormulaCommand::changeType() const {
  return ChangeType::Content;
}

void AddFormulaCommand::redo(Assignment& a) {
  auto* f = a.addFormula(t, afterId);
  if (createdId == -1) {
    createdId = f->id;
  } else {
    t->id = createdId;
  }
}

void AddFormulaCommand::undo(Assignment& a) {
  a.removeFormula(t, createdId);
}

int AddFormulaCommand::resultId() const {
  return createdId;
}

