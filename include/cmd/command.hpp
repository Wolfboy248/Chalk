#pragma once

#include <QString>
#include <memory>
#include <vector>

struct Assignment;
struct Task;
struct Formula;
struct Image;
enum class ChangeType;

struct Command {
  virtual ~Command() = default;
  virtual void undo(Assignment& a) = 0;
  virtual void redo(Assignment& a) = 0;

  virtual int resultId() const { return -1; }
  virtual ChangeType changeType() const = 0;
};

struct CommandManager {
  std::vector<std::unique_ptr<Command>> undoStack;
  std::vector<std::unique_ptr<Command>> redoStack;

  int execute(std::unique_ptr<Command> cmd, Assignment& a) {
    cmd->redo(a);
    int result = cmd->resultId();
    undoStack.push_back(std::move(cmd));
    redoStack.clear();
    return result;
  }

  void undo(Assignment& a) {
    if (undoStack.empty()) return;
    auto cmd = std::move(undoStack.back());
    undoStack.pop_back();
    cmd->undo(a);
    redoStack.push_back(std::move(cmd));
  }

  void redo(Assignment& a) {
    if (redoStack.empty()) return;
    auto cmd = std::move(redoStack.back());
    redoStack.pop_back();
    cmd->redo(a);
    undoStack.push_back(std::move(cmd));
  }
};
