#include <bridge.hpp>

#include <QDebug>

#include <editor.hpp>

void Bridge::receiveLatex(const QString& latex) {
  qDebug() << "Latex: " << latex;
}

void Bridge::refresh() {
  emit refreshed(taskToJson(e->doc()->data().getTask(mTaskId)));
}

void Bridge::addFormula() {
  auto cmd = std::make_unique<AddFormulaCommand>(mTaskId);
  int id = e->doc()->execute(std::move(cmd));

  auto t = e->doc()->data().getTask(id);
  if (!t) return;

  emit focusFormula(id);
  // emit refreshed(taskToJson(t));
  emit taskChanged(taskToJson(t));
}

void Bridge::addFormulaAfter(int afterId) {
  auto cmd = std::make_unique<AddFormulaCommand>(mTaskId, afterId);
  int id = e->doc()->execute(std::move(cmd));

  auto t = e->doc()->data().getTask(id);
  if (!t) return;
  emit focusFormula(id);
  // emit taskChanged(taskToJson(t));
}

void Bridge::removeFormula(int id) {
  // auto t = e->doc()->data().getTask(mTaskId);
  // if (!t) return;
  auto cmd = std::make_unique<RemoveFormulaCommand>(mTaskId, id);
  e->doc()->execute(std::move(cmd));

  // emit taskChanged(taskToJson(e->doc()->data().getTask(mTaskId)));
}

void Bridge::setTask(int id) {
  mTaskId = id;
  if (id == -1) {
    emit taskChanged("null");
    return;
  };
  // emit taskChanged(taskToJson(e->doc()->data().getTask(id)));
}

void Bridge::taskHasChanged() {
  taskChanged(taskToJson(e->doc()->data().getTask(mTaskId)));
}

void Bridge::updateFormula(int id, const QString& latex) {
  if (latex == e->doc()->data().getTask(mTaskId)->getFormula(id)->latex) return;
  qDebug() << "!!! UPDATING FORMULA !!!";
  // if (!mTask) return;
  // for (auto& f : mTask->formulas) {
  //   if (f->id == id) {
  //     f->latex = latex; break;
  //   }
  // }

  auto cmd = std::make_unique<UpdateFormulaCommand>(mTaskId, id, latex);
  int addedId = e->doc()->execute(std::move(cmd));
  // emit evaluateTask(taskToJson(mTask));
}

void Bridge::updateExplanation(int id, const QString& explanation) {
  // if (!mTask) return;
  // for (auto& f : mTask->formulas) {
  //   if (f->id == id) { f->explanation = explanation; break; }
  // }
  // emit updatedExplanation();
}

void Bridge::updateUnitoverride(int id, const QString& unitOverride) {
  // if (!mTask) return;
  // for (auto& f : mTask->formulas) {
  //   if (f->id == id) { f->unitOverride = unitOverride; break; }
  // }
  // emit evaluateTask(taskToJson(mTask));
}

void Bridge::toggleAnswer(int id) {
  // qDebug() << "OMGOMGOMG";
  // if (!mTask) return;
  // for (auto& f : mTask->formulas) {
  //   if (f->id == id) { f->isAnswer = !f->isAnswer; }
  // }
  // emit updatedExplanation();
}

void Bridge::toggleHideAnswer(int id) {
  // qDebug() << "OMGOMGOMG";
  // if (!mTask) return;
  // for (auto& f : mTask->formulas) {
  //   if (f->id == id) { f->hideAnswer = !f->hideAnswer; }
  // }
  // emit updatedExplanation();
}

void Bridge::toggleIntermediate(int id) {
  // qDebug() << "OMGOMGOMG";
  // if (!mTask) return;
  // for (auto& f : mTask->formulas) {
  //   if (f->id == id) { f->isIntermediate = !f->isIntermediate; }
  // }
  // emit taskChanged(taskToJson(mTask));
}

void Bridge::receiveResults(const QString& json) {
  // if (!mTask) return;
  // QJsonArray results = QJsonDocument::fromJson(json.toUtf8()).array();
  // for (const auto& r : results) {
  //   QJsonObject obj = r.toObject();
  //   int id = obj["id"].toInt();
  //   QString result = obj["result"].toString();
  //   QString error = obj["error"].toString();
  //   for (auto& f : mTask->formulas) {
  //     if (f->id == id) {
  //       f->result = result; 
  //       f->error = error;
  //       // qDebug() << f->latex << " == " << f->result;
  //       break;
  //     }
  //   }
  // }
  //
  // emit resultsReady(taskToJson(mTask));
}
