#pragma once

#include "types.hpp"
#include <QApplication>
#include <QWidget>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

class PagesBridge : public QObject {
  Q_OBJECT
public:
  explicit PagesBridge(QObject* parent = nullptr) : QObject(parent) {
    
  }

  void setAssignment(Assignment* a) {
    assignment = a;
  }

public slots:
  void setBg(const QString& str) {
    emit setBgCol(str);
  }

  void jsReady() {
    setBg(
      QApplication::palette().color(QPalette::Window).name()
    );
    emit updatePages(assignmentToJson(assignment));
  }

  void update() {
    emit updatePages(assignmentToJson(assignment));
  }

signals:
  void setBgCol(const QString& str);
  void updatePages(const QString& assignmentJson);

private:
  QString assignmentToJson(Assignment* a) {
    if (!a) return "null";
    QJsonArray tasks;
    for (const auto& t : a->tasks) {
      tasks.append(taskToJson(t.get()));
    }

    return QJsonDocument(QJsonObject{
      {"title", a->title},
      {"tasks", tasks},
    }).toJson();
  }

  QJsonObject taskToJson(Task* task) {
    if (!task) return QJsonObject{};
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

    return QJsonObject{
      {"title", task->title},
      {"formulas", formulas},
    };
  }

  Assignment* assignment = nullptr;
};

