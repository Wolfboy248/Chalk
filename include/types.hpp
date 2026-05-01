#pragma once

#include "changeType.hpp"
#include <QString>
#include <qdebug.h>
#include <vector>
#include <memory>

#include <cmd/addTaskCommand.hpp>
#include <cmd/removeTaskCommand.hpp>
#include <cmd/updateTaskTitleCommand.hpp>
#include <cmd/updateAssignmentTitleCommand.hpp>

struct Image {
  int id;
  QString path;
  QString caption;
};

struct Formula {
  int id;
  QString latex;
  QString explanation;
  QString result;
  QString error;
  QString unitOverride;
  bool isIntermediate = false;
  bool isAnswer = false;
  bool hideAnswer = false;
};

struct Task {
  int id;
  QString title;
  std::vector<std::unique_ptr<Formula>> formulas;
  std::vector<std::unique_ptr<Image>> images;

  Task() = default;

  Task(const Task& other) {
    id = other.id;
    title = other.title;

    formulas.reserve(other.formulas.size());
    for (const auto& f : other.formulas) {
      formulas.push_back(std::make_unique<Formula>(*f));
    }

    images.reserve(other.images.size());
    for (const auto& i : other.images) {
      images.push_back(std::make_unique<Image>(*i));
    }
  }

  Task& operator=(const Task& other) {
    if (this == &other) return *this;

    id = other.id;
    title = other.title;
    formulas.clear();
    images.clear();

    for (const auto& f : other.formulas) {
      formulas.push_back(std::make_unique<Formula>(*f));
    }

    for (const auto& i : other.images) {
      images.push_back(std::make_unique<Image>(*i));
    }

    return *this;
  }
};

struct Assignment {
  QString title = "New Assignment";
  std::vector<std::unique_ptr<Task>> tasks;
  std::vector<QString> names;

  Task* addTask(QString title = "New Task") {
    auto t = std::make_unique<Task>();
    t->id = nextId++;
    t->title = std::move(title);
    auto* ptr = t.get();
    tasks.push_back(std::move(t));
    return ptr;
  }

  Image* addImage(Task* task, const QString& path) {
    auto img = std::make_unique<Image>();
    img->id = nextId++;
    img->path = path;
    auto* ptr = img.get();

    task->images.push_back(std::move(img));
    return ptr;
  }

  void removeImage(Task* task, int id) {
    auto& v = task->images;
    v.erase(
      std::remove_if(v.begin(), v.end(), [id](const auto& img){ return img->id == id; }), v.end()
    );
  }

  Formula* addFormula(Task* task, int afterId = -1) {
    auto f = std::make_unique<Formula>();
    f->id = nextId++;
    auto* ptr = f.get();
    auto& v = task->formulas;
    if (afterId == -1)
      v.push_back(std::move(f));
    else {
      auto it = std::find_if(v.begin(), v.end(), [afterId](const auto& x){ return x->id == afterId; });
      v.insert(it + 1, std::move(f));
    }
    return ptr;
  }

  void removeTask(int id) {
    auto& v = tasks;
    v.erase(
      std::remove_if(v.begin(), v.end(), [id](const auto& t){ return t->id == id; }),
      v.end()
    );
  }

  void removeFormula(Task* task, int id) {
    auto& v = task->formulas;
    v.erase(std::remove_if(v.begin(), v.end(), [id](const auto& f){ return f->id == id; }), v.end());
  }

  void reorderTask(int fromIndex, int toIndex) {
    auto t = std::move(tasks[fromIndex]);
    tasks.erase(tasks.begin() + fromIndex);
    tasks.insert(tasks.begin() + toIndex, std::move(t));
  }

  void reorderFormula(Task* task, int fromIndex, int toIndex) {
    auto& v = task->formulas;
    auto f = std::move(v[fromIndex]);
    v.erase(v.begin() + fromIndex);
    v.insert(v.begin() + toIndex, std::move(f));
  }

private:
  int nextId = 0;
};

