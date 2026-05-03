#include <historyDock.hpp>

#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QLabel>

#include <editor.hpp>

HistoryDock::HistoryDock(QWidget* parent, Editor* editor) 
  : QDockWidget("History", parent), e{editor} {
  setAllowedAreas(Qt::AllDockWidgetAreas);
  setFeatures(
    QDockWidget::DockWidgetMovable |
    QDockWidget::DockWidgetFloatable
  );

  list->setSelectionMode(QAbstractItemView::NoSelection);
  list->setFocusPolicy(Qt::NoFocus);
  list->setWordWrap(true);
  list->setUniformItemSizes(false);

  setWidget(list);
}

void HistoryDock::refresh() {
  list->clear();

  int totalUndos = 0;
  for (int i = 0; i < e->doc()->cmdMgr().undoStack.size(); i++) {
    QWidget* itemWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(itemWidget);

    layout->addWidget(new QLabel("Change #" + QString::number(i) + " (Undo)"));
    layout->addWidget(new QLabel("Type: " + changeTypeToStr(
      e->doc()->cmdMgr().undoStack[i]->changeType()
    )));
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(itemWidget->sizeHint());

    list->addItem(item);
    list->setItemWidget(item, itemWidget);
    totalUndos++;
  }

  for (int i = e->doc()->cmdMgr().redoStack.size() - 1; i >= 0; i--) {
    QWidget* itemWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(itemWidget);

    QLabel* title = new QLabel{"Change #" + QString::number(totalUndos) + " (Redo)"};
    title->setStyleSheet("color: gray;");
    // QPalette pal = title->palette();
    // pal.setColor(QPalette::WindowText, Qt::gray);
    // title->setPalette(pal);
    layout->addWidget(title);

    QLabel* type = new QLabel{"Type: " + changeTypeToStr(
      e->doc()->cmdMgr().redoStack[i]->changeType()
    )};
    type->setStyleSheet("color: gray;");
    // type->setPalette(pal);
    layout->addWidget(type);

    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    QListWidgetItem* item = new QListWidgetItem(list);
    item->setSizeHint(itemWidget->sizeHint());

    list->addItem(item);
    list->setItemWidget(item, itemWidget);
    totalUndos++;
  }
}

