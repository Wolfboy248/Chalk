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

  void scrollToTask(Task* task) {
    QJsonObject t = taskToJson(task);
    QJsonDocument doc(t);
    emit scrollToTask(doc.toJson(QJsonDocument::Compact));
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

  void updateFull() {
    emit updatePagesFull(assignmentToJson(assignment));
  }

  void updateTitle(const QString& title) {
    qDebug() << "Updating title";
    if (!assignment) return;
    assignment->title = title;
  }

  void updateTaskTitle(int id, const QString& title) {
    if (!assignment) return;
    for (auto& t : assignment->tasks) {
      if (t->id == id) {
        t->title = title;
        break;
      }
    }
    emit updatedTaskTitle();
  }

  void removeImage(int taskId, int imageId) {
    if (!assignment) return;
    for (auto& t : assignment->tasks) {
      if (t->id == taskId) {
        assignment->removeImage(t.get(), imageId);
        break;
      }
    }
    emit updatedTaskTitle();
  }

signals:
  void setBgCol(const QString& str);
  void updatePages(const QString& assignmentJson);
  void updatePagesFull(const QString& assignmentJson);

  void scrollToTask(const QString& taskJson);

  void updatedTaskTitle();

private:
  QString assignmentToJson(Assignment* a) {
    if (!a) return "null";
    QJsonArray tasks;
    for (const auto& t : a->tasks) {
      tasks.append(taskToJson(t.get()));
    }

    QJsonArray names;
    for (const auto& n : a->names) {
      names.append(QJsonValue(n));
    }

    return QJsonDocument(QJsonObject{
      {"title", a->title},
      {"tasks", tasks},
      {"names", names},
    }).toJson();
  }

  QJsonObject taskToJson(Task* task) {
    if (!task) return QJsonObject{};
    QJsonArray formulas;
    for (const auto& f : task->formulas) {
      formulas.append(QJsonObject{
        {"id", f->id},
        {"latex", f->latex},
        {"explanation", f->explanation},
        {"result", f->result},
        {"error", f->error},
        {"isAnswer", f->isAnswer},
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

    return QJsonObject{
      {"title", task->title},
      {"id", task->id},
      {"formulas", formulas},
      {"images", images},
    };
  }

  Assignment* assignment = nullptr;
};

