#pragma once

#include "types.hpp"
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

class Bridge : public QObject {
  Q_OBJECT
public:
  explicit Bridge(QObject* parent = nullptr) : QObject(parent) {}

  QString formula() const { return mFormula; }

  void setAssignment(Assignment* a) {
    assignment = a;
  }

  void setTask(Task* task) {
    mTask = task;
    if (!task) return;
    emit taskChanged(taskToJson(task));
  }

public slots:
  void updateFormula(int id, const QString& latex) {
    if (!mTask) return;
    for (auto& f : mTask->formulas) {
      if (f->id == id) {
        f->latex = latex; break;
      }
    }
    emit evaluateTask(taskToJson(mTask));
  }

  void updateExplanation(int id, const QString& explanation) {
    if (!mTask) return;
    for (auto& f : mTask->formulas) {
      if (f->id == id) { f->explanation = explanation; break; }
    }
    emit updatedExplanation();
  }

  void addFormula() {
    if (!mTask) return;
    auto f = assignment->addFormula(mTask);
    emit focusFormula(f->id);
    emit taskChanged(taskToJson(mTask));
  }

  void addFormulaAfter(int id) {
    if (!mTask) return;
    auto f = assignment->addFormula(mTask, id);
    emit focusFormula(f->id);
    emit taskChanged(taskToJson(mTask));
  }

  void removeFormula(int id) {
    if (!mTask) return;

    auto& v = mTask->formulas;
    auto it = std::find_if(v.begin(), v.end(), [id](const auto& f){ return f->id == id; });

    if (it != v.end()) {
      if (it != v.begin()) {
        int prevId = std::prev(it)->get()->id;
        emit focusFormula(prevId);
      } else if (std::next(it) != v.end()) {
        int nextId = std::next(it)->get()->id;
        emit focusFormula(nextId);
      }
    }
    assignment->removeFormula(mTask, id);
    emit taskChanged(taskToJson(mTask));
  }

  void receiveLatex(const QString& latex);

  void receiveResults(const QString& json) {
    if (!mTask) return;
    QJsonArray results = QJsonDocument::fromJson(json.toUtf8()).array();
    for (const auto& r : results) {
      QJsonObject obj = r.toObject();
      int id = obj["id"].toInt();
      QString result = obj["result"].toString();
      QString error = obj["error"].toString();
      for (auto& f : mTask->formulas) {
        if (f->id == id) {
          f->result = result; 
          f->error = error;
          // qDebug() << f->latex << " == " << f->result;
          break;
        }
      }
    }

    emit resultsReady(taskToJson(mTask));
  }

signals:
  void taskChanged(const QString& json);
  void focusFormula(int id);

  void evaluateTask(const QString& json);
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
      });
    }

    return QJsonDocument(QJsonObject{
      {"title", task->title},
      {"formulas", formulas},
    }).toJson();
  }

  Assignment* assignment;
  QString mFormula;
  Task* mTask = nullptr;
  int nextId = 0;
};

