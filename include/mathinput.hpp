#pragma once

#include "types.hpp"
#include <QDockWidget>
#include <QObject>

class Bridge;
class Editor;

class MathInputDock : public QDockWidget {
  Q_OBJECT
public:
  MathInputDock(QWidget* parent = nullptr, Editor* editor = nullptr);
  void setTask(Task* task);

  void setAssignment(Assignment* a);

signals:
  void changed();

private:
  void onFormulaChange();

  Assignment* assignment = nullptr;
  Bridge* bridge = nullptr;
  Task* lastTask = nullptr;

  Editor* e;
};

