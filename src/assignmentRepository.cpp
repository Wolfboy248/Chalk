#include <assignmentRepository.hpp>

#include <xdf/xdf.h>

using namespace xdf;

void AssignmentRepository::save(const Assignment& assignment, const QString& path) {
  XDFRoot root{};
  root.addValue("version", "0.1.0");

  root.addValue("title", assignment.title.toStdString());

  XDFNode tasks{"tasks"};
  int i = 0;
  for (const auto& t : assignment.tasks) {
    XDFNode task{"task_" + std::to_string(t->id)};
    task.addValue("title", t->title.toStdString());
    task.addValue("order", std::to_string(i++));

    XDFNode formulas{"formulas"};
    int j = 0;
    for (const auto& f : t->formulas) {
      XDFNode formula{"formula_" + std::to_string(f->id)};
      formula.addValue("latex", f->latex.toStdString());
      formula.addValue("result", f->result.toStdString());
      formula.addValue("order", std::to_string(j++));
      formulas.append(formula);
    }

    task.append(formulas);
    tasks.append(task);
  }
  root.append(tasks);

  root.write(path.toStdString());
}

Assignment AssignmentRepository::load(const QString& path) {
  Assignment assignment;

  XDFRoot root;
  if (root.read(path.toStdString())) {
    std::string version = root.getValues()["version"];

    std::string title = root.getValues()["title"];
    assignment.title = QString{title.c_str()};

    auto tasks = root.getChildren().find("tasks")->second;
    std::vector<std::pair<int, XDFNode>> tasksOrdered;

    for (auto& [n, task] : tasks.getChildren()) {
      int order = std::stoi(task.getValues()["order"]);
      tasksOrdered.push_back({order, task});
    }
    std::sort(tasksOrdered.begin(), tasksOrdered.end(), [](auto& a, auto& b){ return a.first < b.first; });

    for (auto& [name, task] : tasksOrdered) {
      std::string taskTitle = task.getValues()["title"];

      Task* t = assignment.addTask();
      t->title = QString{taskTitle.c_str()};

      auto formulas = task.getChildren().find("formulas")->second;

      std::vector<std::pair<int, XDFNode>> ordered;
      for (auto& [n, formula] : formulas.getChildren()) {
        int order = std::stoi(formula.getValues()["order"]);
        ordered.push_back({order, formula});
      }
      std::sort(ordered.begin(), ordered.end(), [](auto& a, auto& b){ return a.first < b.first; });

      for (auto& [_, formula] : ordered) {
        std::string latex = formula.getValues()["latex"];
        std::string result = formula.getValues()["result"];
        Formula* f = assignment.addFormula(t);
        f->latex = QString{latex.c_str()};
        f->result = QString{result.c_str()};
      }
    }
  }

  return assignment;
}

