#include <cmd/toggleAnswerCommand.hpp>

#include <types.hpp>

ChangeType ToggleAnswerCommand::changeType() const {
  return ChangeType::Content;
}

void ToggleAnswerCommand::redo(Assignment& a) {
  qDebug("Toggling answer");
  auto f = a.getTask(taskId)->getFormula(formulaId);
  f->isAnswer = !f->isAnswer;
}

void ToggleAnswerCommand::undo(Assignment& a) {
  auto f = a.getTask(taskId)->getFormula(formulaId);
  f->isAnswer = !f->isAnswer;
}

int ToggleAnswerCommand::resultId() const {
  return taskId;
}

