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
  void setTask(int id);

  void refresh();

signals:
  void changed();

private:
  void onFormulaChange();

  Bridge* bridge = nullptr;
  int lastTaskId = -1;
  // Task* lastTask = nullptr;

  Editor* e;
};

