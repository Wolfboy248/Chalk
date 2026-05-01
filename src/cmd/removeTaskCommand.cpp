#include <cmd/removeTaskCommand.hpp>

#include <types.hpp>

ChangeType RemoveTaskCommand::changeType() const {
  return ChangeType::Structure;
}

void RemoveTaskCommand::redo(Assignment& a) {
  auto& v = a.tasks;
  auto it = std::find_if(
    v.begin(),
    v.end(),
    [this](const auto& t){ return t->id == taskId; }
  );
  if (it == v.end()) return;

  index = std::distance(v.begin(), it);
  backup = std::move(*it);
  v.erase(it);
}

void RemoveTaskCommand::undo(Assignment& a) {
  if (!backup) return;
  a.tasks.insert(a.tasks.begin() + index, std::move(backup));
}

int RemoveTaskCommand::resultId() const {
  return taskId;
}

