#include "navigator.hpp"

#include <QToolBar>
#include <QFileDialog>

#include <editor.hpp>

NavigatorWidget::NavigatorWidget(Editor* editor, QWidget* parent) : QDockWidget("Navigator", parent), e{editor} {
  setAllowedAreas(Qt::AllDockWidgetAreas);
  setFeatures(
    QDockWidget::DockWidgetMovable |
    QDockWidget::DockWidgetFloatable
  );

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

// void NavigatorWidget::setAssignment(Assignment* a) {
//   assignment = a;
//   refresh();
// }

void NavigatorWidget::refresh(int selectedId) {
  // if (!assignment) return;
  refreshing = true;
  tree->clear();

  // QTreeWidgetItem* assignmentParent = new QTreeWidgetItem(tree);
  // assignmentParent->setText(0, assignment->title);
  // assignmentParent->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

  for (int i = 0; i < e->doc()->data().tasks.size(); i++) {
    const auto& task = e->doc()->data().tasks[i];
    QTreeWidgetItem* item = new QTreeWidgetItem(tree);
    item->setData(0, Qt::UserRole, task->id);
    item->setText(0, task->title);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);

    if (selectedId != -1 && selectedId == task->id) {
      item->setSelected(true);
      tree->setCurrentItem(item);
      emit taskSelected(e->doc()->data().tasks[i]->id);
    }
  }

  refreshing = false;
}

void NavigatorWidget::setupToolbar() {
  connect(toolbar->addAction("Add"), &QAction::triggered, this, [&]() {
    // if (!assignment) return;
    auto cmd = std::make_unique<AddTaskCommand>("New Task");
    int addedId = e->doc()->execute(std::move(cmd));
    // Task* added = assignment->addTask();
    refresh(addedId);
    emit changed();
  });

  connect(toolbar->addAction("Add Image"), &QAction::triggered, this, [&] () {
    if (!tree->currentItem()) return;
    if (e->doc()->data().tasks.size() == 0) return;
    Task* currentTask = e->doc()->data().tasks[tree->indexOfTopLevelItem(tree->currentItem())].get();
    QString fileName = QFileDialog::getOpenFileName(
      this,
      "Open image",
      "",
      "PNG Files (*.png);; JPG Files (*.jpg);; JPEG Files (*.jpeg)"
    );
    if (fileName == "") return;
    e->doc()->addImage(currentTask, fileName);

    emit changed();
  });

  connect(toolbar->addAction("Remove"), &QAction::triggered, this, [&]() {
    if (!tree->currentItem()) return;
    int removedId = tree->currentItem()->data(0, Qt::UserRole).toInt();
    int removedIndex = tree->indexOfTopLevelItem(tree->currentItem());

    auto cmd = std::make_unique<RemoveTaskCommand>(removedId);
    e->doc()->execute(std::move(cmd));
    
    // assignment->removeTask(removedId);

    if (e->doc()->data().tasks.empty()) {
      emit taskSelected(-1);
      refresh(-1);
    } else {
      int selectIndex = std::min(removedIndex, (int)e->doc()->data().tasks.size() - 1);
      if (selectIndex < 0) selectIndex = 0;
      int nextId = e->doc()->data().tasks[selectIndex]->id;
      refresh(nextId);
    }

    emit changed();
  });

  connect(tree, &QTreeWidget::currentItemChanged, [&](QTreeWidgetItem* current, QTreeWidgetItem*) {
    // qDebug() << "Current changed";
    if (!current) return;
    // qDebug() << "not null";
    int index = tree->indexOfTopLevelItem(current);
    if (index == -1) return;
    // qDebug() << "valid";
    emit taskSelected(e->doc()->data().tasks[index]->id);
  });

  connect(tree, &TaskTreeWidget::orderChanged, [&]() {
    qDebug() << "MOVED!!!";
    // if (!assignment) return;

    std::vector<int> newOrder;
    for (int i = 0; i < tree->topLevelItemCount(); i++) {
      newOrder.push_back(tree->topLevelItem(i)->data(0, Qt::UserRole).toInt());
    }

    std::vector<std::unique_ptr<Task>> reordered;
    for (int id : newOrder) {
      auto it = std::find_if(
        e->doc()->data().tasks.begin(),
        e->doc()->data().tasks.end(),
        [id](const auto& t){ return t->id == id; }
      );
      if (it != e->doc()->data().tasks.end()) {
        if ((*it)->id == tree->currentItem()->data(0, Qt::UserRole).toInt()) {
          emit taskSelected((*it)->id);
        }
        reordered.push_back(std::move(*it));
      }
    }
    e->doc()->data().tasks = std::move(reordered);

    emit changed();
  });

  connect(tree, &QTreeWidget::itemChanged, [&](QTreeWidgetItem* item, int) {
    if (refreshing) return;
    // if (!assignment) return;
    int index = tree->indexOfTopLevelItem(item);
    if (index == -1) return;

    // e->doc()->data().tasks[index]->title = item->text(0);
    auto cmd = std::make_unique<UpdateTaskTitleCommand>(e->doc()->data().tasks[index]->id, item->text(0));
    e->doc()->execute(std::move(cmd));
    emit changed();
  });
}

