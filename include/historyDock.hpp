#pragma once

#include <QDockWidget>
#include <QObject>
#include <QListWidgetItem>

class Editor;

class HistoryDock : public QDockWidget {
  Q_OBJECT
public:
  HistoryDock(QWidget* parent = nullptr, Editor* editor = nullptr);

  void refresh();

  QSize sizeHint() const override { return QSize{400, 600}; }

private:
  QListWidget* list = new QListWidget(this);
  Editor* e;
};

