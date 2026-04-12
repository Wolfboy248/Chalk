#include "navigator.hpp"

#include <QToolBar>

#include <editor.hpp>

NavigatorWidget::NavigatorWidget(QWidget* parent) : QDockWidget("Navigator", parent) {
  QWidget* container = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  toolbar = new QToolBar(this);
  layout->addWidget(toolbar);

  tree = new TaskTreeWidget(this);
  tree->setHeaderHidden(true);
  tree->setDragDropMode(QAbstractItemView::InternalMove);
  tree->setDefaultDropAction(Qt::MoveAction);
  tree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  setupToolbar();
  layout->addWidget(tree);

  setWidget(container);
}

void NavigatorWidget::setAssignment(Assignment* a) {
  assignment = a;
  refresh();
}

void NavigatorWidget::refresh(int selectedId) {
  if (!assignment) return;
  refreshing = true;
  tree->clear();

  // QTreeWidgetItem* assignmentParent = new QTreeWidgetItem(tree);
  // assignmentParent->setText(0, assignment->title);
  // assignmentParent->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

  for (int i = 0; i < assignment->tasks.size(); i++) {
    const auto& task = assignment->tasks[i];
    QTreeWidgetItem* item = new QTreeWidgetItem(tree);
    item->setData(0, Qt::UserRole, task->id);
    item->setText(0, task->title);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);

    if (selectedId != -1 && selectedId == task->id) {
      item->setSelected(true);
      tree->setCurrentItem(item);
      emit taskSelected(assignment->tasks[i].get());
    }
  }

  refreshing = false;
}

void NavigatorWidget::setupToolbar() {
  connect(toolbar->addAction("Add"), &QAction::triggered, this, [&]() {
    if (!assignment) return;
    Task* added = assignment->addTask();
    refresh(added->id);
    emit changed();
  });

  connect(toolbar->addAction("Remove"), &QAction::triggered, this, [&]() {
    if (!assignment || !tree->currentItem()) return;
    int removedId = tree->currentItem()->data(0, Qt::UserRole).toInt();
    int removedIndex = tree->indexOfTopLevelItem(tree->currentItem());
    
    assignment->removeTask(removedId);

    if (assignment->tasks.empty()) {
      emit taskSelected(nullptr);
      refresh(-1);
    } else {
      int selectIndex = std::min(removedIndex, (int)assignment->tasks.size() - 1);
      if (selectIndex < 0) selectIndex = 0;
      int nextId = assignment->tasks[selectIndex]->id;
      refresh(nextId);
    }

    emit changed();
  });

  connect(tree, &QTreeWidget::currentItemChanged, [&](QTreeWidgetItem* current, QTreeWidgetItem*) {
    // qDebug() << "Current changed";
    if (!current || !assignment) return;
    // qDebug() << "not null";
    int index = tree->indexOfTopLevelItem(current);
    if (index == -1) return;
    // qDebug() << "valid";
    emit taskSelected(assignment->tasks[index].get());
  });

  connect(tree, &TaskTreeWidget::orderChanged, [&]() {
    qDebug() << "MOVED!!!";
    if (!assignment) return;

    std::vector<int> newOrder;
    for (int i = 0; i < tree->topLevelItemCount(); i++) {
      newOrder.push_back(tree->topLevelItem(i)->data(0, Qt::UserRole).toInt());
    }

    std::vector<std::unique_ptr<Task>> reordered;
    for (int id : newOrder) {
      auto it = std::find_if(
        assignment->tasks.begin(),
        assignment->tasks.end(),
        [id](const auto& t){ return t->id == id; }
      );
      if (it != assignment->tasks.end()) {
        if ((*it)->id == tree->currentItem()->data(0, Qt::UserRole).toInt()) {
          emit taskSelected((*it).get());
        }
        reordered.push_back(std::move(*it));
      }
    }
    assignment->tasks = std::move(reordered);

    // std::vector<Task> reordered;
    // for (int i = 0; i < tree->topLevelItemCount(); i++) {
    //   int originalIndex = tree->topLevelItem(i)->data(0, Qt::UserRole).toInt();
    //   assignment->reorderTask(originalIndex, i);
    //   // reordered.push_back(assignment->tasks[originalIndex]);
    // }
    // assignment->tasks = reordered;

    // for (int i = 0; i < tree->topLevelItemCount(); i++) {
    //   tree->topLevelItem(i)->setData(0, Qt::UserRole, i);
    // }

    emit changed();
  });

  connect(tree, &QTreeWidget::itemChanged, [&](QTreeWidgetItem* item, int) {
    if (refreshing) return;
    if (!assignment) return;
    int index = tree->indexOfTopLevelItem(item);
    if (index == -1) return;
    assignment->tasks[index]->title = item->text(0);
    emit changed();
  });
}

