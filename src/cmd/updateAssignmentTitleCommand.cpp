#include <cmd/updateAssignmentTitleCommand.hpp>

#include <types.hpp>

UpdateAssignmentTitleCommand::UpdateAssignmentTitleCommand(QString t)
  : newTitle{t} {}

ChangeType UpdateAssignmentTitleCommand::changeType() const {
  return ChangeType::Metadata;
}

void UpdateAssignmentTitleCommand::redo(Assignment& a) {
  oldTitle = a.title;
  a.title = newTitle;
}

void UpdateAssignmentTitleCommand::undo(Assignment& a) {
  a.title = oldTitle;
}
