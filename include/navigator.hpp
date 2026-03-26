#pragma once

#include "types.hpp"
#include "treewidget.hpp"
#include <QDockWidget>
#include <QObject>
#include <QTreeWidget>
#include <QToolBar>

class NavigatorWidget : public QDockWidget {
  Q_OBJECT
public:
  NavigatorWidget(QWidget* parent = nullptr);

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

  bool refreshing = false;
};

