#include <cmd/addFormulaCommand.hpp>

#include <types.hpp>

AddFormulaCommand::AddFormulaCommand(Task* task, int aId)
  : taskId{task->id}, afterId{aId} {}

ChangeType AddFormulaCommand::changeType() const {
  return ChangeType::Content;
}

void AddFormulaCommand::redo(Assignment& a) {
  // auto* f = a.addFormula(t, afterId);
  auto& v = a.tasks;
  auto it = std::find_if(v.begin(), v.end(), [this](const auto& t){ return t->id == taskId; });
  if (it == v.end()) {
    qDebug() << "AddFormulaCommand::redo: task not found. Id: " + QString::number(taskId);
    return;
  }
  Task* task = it->get();
  auto* f = a.addFormula(task, afterId);
  if (createdId == -1) {
    qDebug() << "New formula";
    createdId = f->id;
  } else {
    qDebug() << "Old getting redone";
    f->id = createdId;
  }

  // if (createdId == -1) {
  //   qDebug() << "New formula";
  //   createdId = f->id;
  // } else {
  //   qDebug() << "Old getting redone";
  //   t->id = createdId;
  // }
}

void AddFormulaCommand::undo(Assignment& a) {
  qDebug() << "New formula getting undone (removeFormula)";
  auto& v = a.tasks;
  auto it = std::find_if(v.begin(), v.end(), [this](const auto& t){ return t->id == taskId; });
  if (it == v.end()) {
    qDebug() << "AddFormulaCommand::undo: task not found. Id: " + QString::number(taskId);
    return;
  }
  // a.removeFormula(it->get(), createdId);
  //
  // qDebug() << "New formula getting undone (removeFormula)";
  // a.removeFormula(t, createdId);
}

int AddFormulaCommand::resultId() const {
  return createdId;
}

