#include <documentModel.hpp>

#include <assignmentRepository.hpp>

#include <QDebug>

DocumentModel::DocumentModel(QObject* parent) : QObject(parent) {}

// === Mutating the assignment ==
int DocumentModel::execute(std::unique_ptr<Command> cmd) {
  ChangeType type = cmd->changeType();
  int id = mCommandManager.execute(std::move(cmd), mAssignment);
  notifyChange(type);
  return id;
}

void DocumentModel::undo() {
  if (mCommandManager.undoStack.empty()) return;

  ChangeType type = mCommandManager.undoStack.back()->changeType();
  mCommandManager.undo(mAssignment);
  notifyChange(type);
}

void DocumentModel::redo() {
  if (mCommandManager.redoStack.empty()) return;

  ChangeType type = mCommandManager.redoStack.back()->changeType();
  mCommandManager.redo(mAssignment);
  notifyChange(type);
}

// === TEMPORARY WRAPPERS ===
// This should all be removed and put into the command system. You cannot
// undo/redo any of this
Task* DocumentModel::addTask(const QString& title) {
  auto* t = mAssignment.addTask(title);
  notifyChange(ChangeType::Structure);
  return t;
}

void DocumentModel::removeTask(int id) {
  mAssignment.removeTask(id);
  notifyChange(ChangeType::Structure);
}

Formula* DocumentModel::addFormula(Task* task, int afterId) {
  auto* f = mAssignment.addFormula(task, afterId);
  notifyChange(ChangeType::Structure);
  return f;
}

void DocumentModel::removeFormula(Task* task, int id) {
  mAssignment.removeFormula(task, id);
  notifyChange(ChangeType::Structure);
}

Image* DocumentModel::addImage(Task* task, const QString& path) {
  auto* img = mAssignment.addImage(task, path);
  notifyChange(ChangeType::Structure);
  return img;
}

void DocumentModel::removeImage(Task* task, int id) {
  mAssignment.removeImage(task, id);
  notifyChange(ChangeType::Structure);
}

void DocumentModel::updateFormula(Formula* f, const Formula& newValues) {
  *f = newValues;
  f->id = newValues.id;
  notifyChange(ChangeType::Content);
}

void DocumentModel::updateTaskTitle(Task* t, const QString& title) {
  t->title = title;
  notifyChange(ChangeType::Structure); // navigator needs to refresh
}

void DocumentModel::updateAssignmentTitle(const QString& title) {
  mAssignment.title = title;
  notifyChange(ChangeType::Metadata);
}

void DocumentModel::updateNames(const std::vector<QString>& names) {
  mAssignment.names = names;
  notifyChange(ChangeType::Metadata);
}

void DocumentModel::selectedTaskChanged() {
  notifyChange(ChangeType::Selection);
}

// === Saving/loading stuff ===
void DocumentModel::setCurrentFile(const QString& path) {
  mCurrentFile = path;
  emit fileChanged(path);
}

bool DocumentModel::save() {
  if (mCurrentFile.isEmpty()) return false;

  AssignmentRepository::save(mAssignment, mCurrentFile);
  markClean();
  return true;
}

bool DocumentModel::saveAs(const QString& path) {
  AssignmentRepository::save(mAssignment, path);
  setCurrentFile(path);
  markClean();
  return true;
}

bool DocumentModel::load(const QString& path) {
  mAssignment = AssignmentRepository::load(path);

  setCurrentFile(path);
  notifyChange(ChangeType::Structure);
  markClean();
  return true;
}

void DocumentModel::reset() {
  mAssignment = Assignment{};
  setCurrentFile("");
  mCommandManager.undoStack.clear();
  mCommandManager.redoStack.clear();
  notifyChange(ChangeType::Structure);
  markClean();
}

// === PRIVATE ===
void DocumentModel::notifyChange(ChangeType type) {
  if (type != ChangeType::Selection) markDirty();
  emit changed(type);
}

void DocumentModel::markDirty() {
  if (!mUnsaved) {
    mUnsaved = true;
    emit saveStateChanged(true);
  }
}

void DocumentModel::markClean() {
  mUnsaved = false;
  emit saveStateChanged(false);
}

