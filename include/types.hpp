#pragma once

#include <QString>
#include <vector>
#include <memory>

struct Formula {
  int id;
  QString latex;
  QString explanation;
  QString result;
  QString error;
};

struct Task {
  int id;
  QString title;
  std::vector<std::unique_ptr<Formula>> formulas;
};

struct Assignment {
  QString title = "New Assignment";
  std::vector<std::unique_ptr<Task>> tasks;

  Task* addTask(QString title = "New Task") {
    auto t = std::make_unique<Task>();
    t->id = nextId++;
    t->title = std::move(title);
    auto* ptr = t.get();
    tasks.push_back(std::move(t));
    return ptr;
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

