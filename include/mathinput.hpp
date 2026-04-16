#pragma once

#include "bridge.hpp"
#include "types.hpp"
#include <QDockWidget>
#include <QObject>

class MathInputDock : public QDockWidget {
  Q_OBJECT
public:
  MathInputDock(QWidget* parent = nullptr);
  void setTask(Task* task);

  void setAssignment(Assignment* a) {
    bridge->setAssignment(a);
    assignment = a;
  }

signals:
  void changed();

private:
  void onFormulaChange();

  Assignment* assignment = nullptr;
  Bridge* bridge = nullptr;
  Task* lastTask = nullptr;
};

