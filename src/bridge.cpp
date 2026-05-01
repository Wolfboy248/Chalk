#include <bridge.hpp>

#include <QDebug>

#include <editor.hpp>

void Bridge::receiveLatex(const QString& latex) {
  qDebug() << "Latex: " << latex;
}

void Bridge::addFormula() {
  if (!mTask) return;
  auto cmd = std::make_unique<AddFormulaCommand>(mTask);
  int id = e->doc()->execute(std::move(cmd));

  emit focusFormula(id);
  emit taskChanged(taskToJson(mTask));
}

void Bridge::addFormulaAfter(int afterId) {
  if (!mTask) return;
  auto cmd = std::make_unique<AddFormulaCommand>(mTask, afterId);
  int id = e->doc()->execute(std::move(cmd));

  emit focusFormula(id);
  emit taskChanged(taskToJson(mTask));
}

void Bridge::removeFormula(int id) {
  if (!mTask) return;

  auto cmd = std::make_unique<RemoveFormulaCommand>(mTask->id, id);
  e->doc()->execute(std::move(cmd));

  emit taskChanged(taskToJson(mTask));
}
