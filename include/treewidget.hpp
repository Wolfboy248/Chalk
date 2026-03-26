#pragma once

#include <QTreeWidget>
#include <QObject>
#include <QDropEvent>
#include <QPainter>

class TaskTreeWidget : public QTreeWidget {
  Q_OBJECT
public:
  TaskTreeWidget(QWidget* parent = nullptr) : QTreeWidget(parent) {
    setDropIndicatorShown(true);
  }
signals:
  void orderChanged();
protected:
  void dropEvent(QDropEvent* event) override {
    QList<QTreeWidgetItem*> dragged = selectedItems();
    // QTreeWidgetItem* target = itemAt(event->position().toPoint());
    auto indicator = dropIndicatorPosition();

    for (auto* item : dragged) {
      bool isTask = item->parent() == nullptr; // top-level = task

      if (isTask) {
        if (indicator == QAbstractItemView::OnItem) {
          event->ignore();
          return;
        }
      } else {
        // Formula
      }
    }
    QTreeWidget::dropEvent(event);

    for (auto* item : dragged) {
      item->setSelected(true);
      setCurrentItem(item);
    }

    emit orderChanged();
  }

private:
  QTreeWidgetItem* dropTargetItem = nullptr;
  bool dropAbove = false;
};

