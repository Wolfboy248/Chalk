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

  void refresh();

signals:
  void changed();

private:
  void onFormulaChange();

  Bridge* bridge = nullptr;
  Task* lastTask = nullptr;

  Editor* e;
};

