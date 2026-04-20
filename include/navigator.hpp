#pragma once

#include "types.hpp"
#include "treewidget.hpp"
#include <QDockWidget>
#include <QObject>
#include <QTreeWidget>
#include <QToolBar>

class Editor;

class NavigatorWidget : public QDockWidget {
  Q_OBJECT
public:
  NavigatorWidget(Editor* e, QWidget* parent = nullptr);

  void setAssignment(Assignment* a);
  void refresh(int selectedId = -1);

signals:
  void changed();
  void taskSelected(Task* task);
private:
  void setupToolbar();

  Assignment* assignment = nullptr;

  QToolBar* toolbar;
  TaskTreeWidget* tree;

  Editor* e;

  bool refreshing = false;
};

