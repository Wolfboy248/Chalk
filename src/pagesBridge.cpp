#include <pagesBridge.hpp>

#include <editor.hpp>

PagesBridge::PagesBridge(QObject* parent, Editor* editor) 
  : QObject{parent}, e{editor} {}

void PagesBridge::jsReady() {
  setBg(
    QApplication::palette().color(QPalette::Window).name()
  );
  updateFull();
  // emit updatePages(assignmentToJson(e->doc()->data()));
}

void PagesBridge::update() {
  emit updatePages(assignmentToJson(e->doc()->data()));
}

void PagesBridge::updateFull() {
  emit updatePagesFull(assignmentToJson(e->doc()->data()));
}

void PagesBridge::removeImage(int taskId, int imageId) {
  // if (!assignment) return;
  for (auto& t : e->doc()->data().tasks) {
    if (t->id == taskId) {
      // TODO: Implement remove image command
      // assignment->removeImage(t.get(), imageId);
      break;
    }
  }
  // emit updatePages(assignmentToJson(e->doc()->data()));
  // emit updatePages(assignmentToJson(assignment));
}

void PagesBridge::updateTitle(const QString& title) {
  qDebug() << "Recieved updateTitle from JS";
  auto cmd = std::make_unique<UpdateAssignmentTitleCommand>(title);
  e->doc()->execute(std::move(cmd));
}

void PagesBridge::updateTaskTitle(int id, const QString& title) {
  qDebug() << "Recieved updateTaskTitle from JS";
  auto cmd = std::make_unique<UpdateTaskTitleCommand>(id, title);
  e->doc()->execute(std::move(cmd));
}

