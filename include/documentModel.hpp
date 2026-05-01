#pragma once

#include <QObject>
#include <memory>
#include <types.hpp>

class DocumentModel : public QObject {
  Q_OBJECT
public:
  explicit DocumentModel(QObject* parent = nullptr);

  const Assignment& data() const { return mAssignment; }
  Assignment& data() { return mAssignment; }
  bool isUnsaved() const { return mUnsaved; }
  const QString& currentFile() const { return mCurrentFile; }

  // === Mutating the assignment ===
  int execute(std::unique_ptr<Command> cmd);

  void undo();
  void redo();

  // TEMPORARY WRAPPERS!! MOVE THIS INTO COMMANDS!!
  Task* addTask(const QString& title = "New Task");
  void removeTask(int id);
  Formula* addFormula(Task* task, int afterId = -1);
  void removeFormula(Task* task, int id);
  Image* addImage(Task* task, const QString& path);
  void removeImage(Task* task, int id);

  void updateFormula(Formula* f, const Formula& newValues);
  void updateTaskTitle(Task* t, const QString& title);
  void updateAssignmentTitle(const QString& title);
  void updateNames(const std::vector<QString>& names);

  // Saving/loading stuff
  void setCurrentFile(const QString& path);
  bool save();
  bool saveAs(const QString& path);
  bool load(const QString& path);
  void reset();

signals:
  void changed(ChangeType type);

  void saveStateChanged(bool unsaved);
  void fileChanged(const QString& path); // For window title

private:
  void notifyChange(ChangeType type);
  void markDirty();
  void markClean();

private:
  Assignment mAssignment;
  CommandManager mCommandManager;
  QString mCurrentFile;
  bool mUnsaved = false;
};

