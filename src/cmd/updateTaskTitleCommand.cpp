#include <cmd/updateTaskTitleCommand.hpp>

#include <types.hpp>

UpdateTaskTitleCommand::UpdateTaskTitleCommand(int id, QString t) 
  : newTitle{t}, taskId{id} {
  
}

ChangeType UpdateTaskTitleCommand::changeType() const {
  return ChangeType::Structure;
}

void UpdateTaskTitleCommand::redo(Assignment& a) {
  auto& v = a.tasks;
  auto it = std::find_if(
    v.begin(),
    v.end(),
    [this](const auto& t){ return t->id == taskId; }
  );
  if (it == v.end()) return;

  oldTitle = it->get()->title;
  qDebug() << "Update task title: " + oldTitle + " -> " + newTitle;
  it->get()->title = newTitle;
}

void UpdateTaskTitleCommand::undo(Assignment& a) {
  auto& v = a.tasks;
  auto it = std::find_if(
    v.begin(),
    v.end(),
    [this](const auto& t){ return t->id == taskId; }
  );
  if (it == v.end()) return;
  it->get()->title = oldTitle;
}

int UpdateTaskTitleCommand::resultId() const {
  return taskId;
}

