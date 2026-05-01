#include <cmd/addTaskCommand.hpp>

#include <types.hpp>

ChangeType AddTaskCommand::changeType() const {
  return ChangeType::Structure;
}

void AddTaskCommand::redo(Assignment& a) {
  auto* t = a.addTask(title);
  createdId = t->id;
}

void AddTaskCommand::undo(Assignment& a) {
  a.removeTask(createdId);
}

int AddTaskCommand::resultId() const {
  return createdId;
}
