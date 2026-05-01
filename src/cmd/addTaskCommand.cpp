#include <cmd/addTaskCommand.hpp>

#include <types.hpp>

ChangeType AddTaskCommand::changeType() const {
  return ChangeType::Structure;
}

void AddTaskCommand::redo(Assignment& a) {
  auto* t = a.addTask(title);
  if (createdId == -1) {
    createdId = t->id;
  } else {
    t->id = createdId;
  }
}

void AddTaskCommand::undo(Assignment& a) {
  a.removeTask(createdId);
}

int AddTaskCommand::resultId() const {
  return createdId;
}
