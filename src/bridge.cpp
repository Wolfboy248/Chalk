#include <bridge.hpp>
#include <editor.hpp>

void Bridge::setTask(int id) {
  mTaskId = id;
  refresh();
}

void Bridge::refresh() {
  auto task = e->doc()->data().getTask(mTaskId);
  emit taskChanged(taskToJson(task));
}

// === Editing Formula ===
void Bridge::updateFormula(int id, const QString &latex) {
  auto task = e->doc()->data().getTask(mTaskId);
  if (!task)
    return;

  auto formula = task->getFormula(id);
  if (formula && formula->latex == latex)
    return;

  e->doc()->execute(std::make_unique<UpdateFormulaCommand>(mTaskId, id, latex));
}

void Bridge::undo() {
  e->doc()->undo();
}

void Bridge::redo() {
  e->doc()->redo();
}

void Bridge::addFormula() {
  int newId = e->doc()->execute(std::make_unique<AddFormulaCommand>(mTaskId));
  refresh();
  emit focusFormula(newId);
}

void Bridge::addFormulaAfter(int afterId) {
  int newId =
      e->doc()->execute(std::make_unique<AddFormulaCommand>(mTaskId, afterId));
  refresh();
  emit focusFormula(newId);
}

void Bridge::removeFormula(int id) {
  e->doc()->execute(std::make_unique<RemoveFormulaCommand>(mTaskId, id));
  refresh();
}

void Bridge::toggleAnswer(int id) {
  qDebug() << "Got toggleAnswer from js!";
  auto task = e->doc()->data().getTask(mTaskId);
  if (!task) return;

  auto formula = task->getFormula(id);
  if (formula) {
    e->doc()->execute(std::make_unique<ToggleAnswerCommand>(mTaskId, id));
    refresh();
  }
}

QString Bridge::taskToJson(Task *task) {
  if (!task)
    return "null";
  QJsonArray formulas;
  for (const auto &f : task->formulas) {
    formulas.append(QJsonObject{
      {"id", f->id},
      {"latex", f->latex},
      {"isAnswer", f->isAnswer},
    });
  }
  return QJsonDocument(QJsonObject{{"id", task->id}, {"formulas", formulas}})
      .toJson();
}
