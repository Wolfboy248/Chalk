#pragma once

#include "types.hpp"
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

class Editor;

class Bridge : public QObject {
  Q_OBJECT
public:
  explicit Bridge(QObject* parent = nullptr, Editor* editor = nullptr) : QObject(parent), e{editor} {}

  QString formula() const { return mFormula; }

  // void setAssignment(Assignment* a) {
  //   assignment = a;
  // }

  void setTask(int id);

  void taskHasChanged();

public slots:
  void updateFormula(int id, const QString& latex);

  void updateExplanation(int id, const QString& explanation);

  void updateUnitoverride(int id, const QString& unitOverride);

  void toggleAnswer(int id);

  void toggleHideAnswer(int id);

  void toggleIntermediate(int id);

  void addFormulaAfter(int id);
  void addFormula();

  void removeFormula(int id);

  void receiveLatex(const QString& latex);

  void receiveResults(const QString& json);

signals:
  void taskChanged(const QString& json);
  void focusFormula(int id);

  // void evaluateTask(const QString& json);
  void resultsReady(const QString& json);

  void updatedExplanation();

private:
  QString taskToJson(Task* task) {
    if (!task) return "null";
    QJsonArray formulas;
    for (const auto& f : task->formulas) {
      formulas.append(QJsonObject{
        {"id", static_cast<int>(f->id)},
        {"latex", f->latex},
        {"explanation", f->explanation},
        {"result", f->result},
        {"error", f->error},
        {"isAnswer", f->isAnswer},
        {"isIntermediate", f->isIntermediate},
        {"unitOverride", f->unitOverride},
        {"hideAnswer", f->hideAnswer},
      });
    }

    QJsonArray images;
    for (const auto& i : task->images) {
      images.append(QJsonObject{
        {"id", static_cast<int>(i->id)},
        {"path", i->path},
        {"caption", i->caption},
      });
    }

    return QJsonDocument(QJsonObject{
      {"title", task->title},
      {"formulas", formulas},
      {"images", images},
    }).toJson();
  }

  // Assignment* assignment;
  QString mFormula;
  // Task* mTask = nullptr;
  int mTaskId = -1;
  int nextId = 0;

  Editor* e;
};

