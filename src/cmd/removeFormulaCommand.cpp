#include <cmd/removeFormulaCommand.hpp>

#include <types.hpp>

ChangeType RemoveFormulaCommand::changeType() const {
  return ChangeType::Content;
}

void RemoveFormulaCommand::redo(Assignment& a) {
  auto& t = a.tasks;
  auto tIt = std::find_if(
    t.begin(),
    t.end(),
    [this](const auto& t){ return t->id == taskId; }
  );
  if (tIt == t.end()) {
    qDebug() << "Task not found (redo). Id: " + QString::number(taskId);
    return;
  }

  auto& f = tIt->get()->formulas;
  auto fIt = std::find_if(
    f.begin(),
    f.end(),
    [this](const auto& f){ return f->id == formulaId; }
  );
  if (fIt == f.end()) {
    qDebug() << "Formula not found (redo). TaskId: " + QString::number(taskId) 
      + ", FormulaId: " + QString::number(formulaId);
    return;
  }

  index = std::distance(f.begin(), fIt);
  backup = std::move(*fIt);
  f.erase(fIt);
}

void RemoveFormulaCommand::undo(Assignment& a) {
  if (!backup) return;
  auto& t = a.tasks;
  auto tIt = std::find_if(
    t.begin(),
    t.end(),
    [this](const auto& t){ return t->id == taskId; }
  );
  if (tIt == t.end()) {
    qDebug() << "Task not found (undo). Id: " + QString::number(taskId);
    return;
  }

  auto& f = tIt->get()->formulas;
  f.insert(f.begin() + index, std::move(backup));
}

int RemoveFormulaCommand::resultId() const {
  return formulaId;
}
