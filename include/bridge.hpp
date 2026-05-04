#pragma once
#include <QObject>
#include <types.hpp>

class Editor;

class Bridge : public QObject {
  Q_OBJECT
public:
  explicit Bridge(QObject *parent, Editor *editor)
      : QObject(parent), e{editor} {}

  void setTask(int id);
  void refresh();

  void lostWindowFocus() {
    emit lostFocus();
  }

public slots:
  // Editing formula
  void updateFormula(int id, const QString &latex);
  void addFormula();
  void addFormulaAfter(int afterId);
  void removeFormula(int id);
  void toggleAnswer(int id);

  void undo();
  void redo();

signals:
  void taskChanged(const QString &json);
  void focusFormula(int id);

  void lostFocus();

private:
  QString taskToJson(Task *task);
  Editor *e;
  int mTaskId = -1;
};
