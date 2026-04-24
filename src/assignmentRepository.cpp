#include <assignmentRepository.hpp>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// #include <xdf/xdf.h>

// using namespace xdf;

void AssignmentRepository::save(const Assignment& assignment, const QString& path) {
  QJsonObject root;
  root["version"] = "0.3.0";
  root["title"] = assignment.title;

  QJsonArray namesArray;
  for (const auto& n : assignment.names) {
    namesArray.append(n);
  }
  root["names"] = namesArray;

  QJsonArray tasksArray;

  for (const auto& t : assignment.tasks) {
    QJsonObject taskObj;
    taskObj["id"] = t->id;
    taskObj["title"] = t->title;

    QJsonArray formulasArray;
    for (const auto& f : t->formulas) {
      QJsonObject fObj;
      fObj["id"] = f->id;
      fObj["latex"] = f->latex;
      fObj["explanation"] = f->explanation;
      fObj["unitOverride"] = f->unitOverride;
      fObj["isAnswer"] = f->isAnswer;
      fObj["hideAnswer"] = f->hideAnswer;
      fObj["isIntermediate"] = f->isIntermediate;
      fObj["result"] = f->result;
      formulasArray.append(fObj);
    }
    taskObj["formulas"] = formulasArray;

    QJsonArray imagesArray;
    for (const auto& img : t->images) {
      QJsonObject iObj;
      iObj["id"] = img->id;
      iObj["path"] = img->path;
      iObj["caption"] = img->caption;
      imagesArray.append(iObj);
    }
    taskObj["images"] = imagesArray;

    tasksArray.append(taskObj);
  }

  root["tasks"] = tasksArray;

  QFile file(path);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(QJsonDocument(root).toJson());
    file.close();
  }
}

Assignment AssignmentRepository::load(const QString& path) {
  Assignment assignment;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return assignment;

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  if (!doc.isObject()) return assignment;

  QJsonObject root = doc.object();

  assignment.title = root["title"].toString();

  for (const auto& n : root["names"].toArray()) {
    assignment.names.push_back(n.toString());
  }

  for (const auto& tVal : root["tasks"].toArray()) {
    QJsonObject tObj = tVal.toObject();

    Task* t = assignment.addTask();
    t->title = tObj["title"].toString();

    for (const auto& fVal : tObj["formulas"].toArray()) {
      QJsonObject fObj = fVal.toObject();

      Formula* f = assignment.addFormula(t);
      f->latex = fObj["latex"].toString();
      f->result = fObj["result"].toString();
      f->explanation = fObj["explanation"].toString();
      f->unitOverride = fObj["unitOverride"].toString();
      f->isAnswer = fObj["isAnswer"].toBool();
      f->hideAnswer = fObj["hideAnswer"].toBool();
      f->isIntermediate = fObj["isIntermediate"].toBool();
    }

    for (const auto& iVal : tObj["images"].toArray()) {
      QJsonObject iObj = iVal.toObject();

      Image* i = assignment.addImage(t, iObj["path"].toString());
      i->caption = iObj["caption"].toString();
    }
  }

  return assignment;
}

// void AssignmentRepository::save(const Assignment& assignment, const QString& path) {
//   XDFRoot root{};
//   root.addValue("version", "0.2.0");
//
//   root.addValue("title", assignment.title.toStdString());
//
//   XDFNode names{"names"};
//   int idx = 0;
//   for (const auto& n : assignment.names) {
//     names.addValue("name_" + std::to_string(idx), n.toStdString());
//     idx++;
//   }
//   root.append(names);
//
//   XDFNode tasks{"tasks"};
//   int i = 0;
//   for (const auto& t : assignment.tasks) {
//     XDFNode task{"task_" + std::to_string(t->id)};
//     task.addValue("title", t->title.toStdString());
//     task.addValue("order", std::to_string(i++));
//
//     XDFNode formulas{"formulas"};
//     int j = 0;
//     for (const auto& f : t->formulas) {
//       XDFNode formula{"formula_" + std::to_string(f->id)};
//       formula.addValue("latex", f->latex.toStdString());
//       formula.addValue("explanation", f->explanation.toStdString());
//       formula.addValue("unitOverride", f->unitOverride.toStdString());
//       formula.addValue("isAnswer", std::to_string(f->isAnswer));
//       formula.addValue("hideAnswer", std::to_string(f->hideAnswer));
//       formula.addValue("isIntermediate", std::to_string(f->isIntermediate));
//       formula.addValue("result", f->result.toStdString());
//       formula.addValue("order", std::to_string(j++));
//       formulas.append(formula);
//     }
//
//     XDFNode images{"images"};
//     int k = 0;
//     for (const auto& i : t->images) {
//       XDFNode image{"image_" + std::to_string(i->id)};
//       image.addValue("path", i->path.toStdString());
//       image.addValue("caption", i->caption.toStdString());
//       image.addValue("order", std::to_string(k++));
//       images.append(image);
//     }
//
//     task.append(formulas);
//     task.append(images);
//     tasks.append(task);
//   }
//   root.append(tasks);
//
//   root.write(path.toStdString());
// }
//
// Assignment AssignmentRepository::load(const QString& path) {
//   Assignment assignment;
//
//   XDFRoot root;
//   if (root.read(path.toStdString())) {
//     std::string version = root.getValues()["version"];
//
//     std::string title = root.getValues()["title"];
//     assignment.title = QString{title.c_str()};
//
//     for (auto& [_, n] : root.getChildren().find("names")->second.getValues()) {
//       assignment.names.push_back(QString{n.c_str()});
//     }
//
//     auto tasks = root.getChildren().find("tasks")->second;
//     std::vector<std::pair<int, XDFNode>> tasksOrdered;
//
//     for (auto& [n, task] : tasks.getChildren()) {
//       int order = std::stoi(task.getValues()["order"]);
//       tasksOrdered.push_back({order, task});
//     }
//     std::sort(tasksOrdered.begin(), tasksOrdered.end(), [](auto& a, auto& b){ return a.first < b.first; });
//
//     for (auto& [name, task] : tasksOrdered) {
//       std::string taskTitle = task.getValues()["title"];
//
//       Task* t = assignment.addTask();
//       t->title = QString{taskTitle.c_str()};
//
//       auto formulas = task.getChildren().find("formulas")->second;
//
//       std::vector<std::pair<int, XDFNode>> ordered;
//       for (auto& [n, formula] : formulas.getChildren()) {
//         int order = std::stoi(formula.getValues()["order"]);
//         ordered.push_back({order, formula});
//       }
//       std::sort(ordered.begin(), ordered.end(), [](auto& a, auto& b){ return a.first < b.first; });
//
//       for (auto& [_, formula] : ordered) {
//         std::string latex = formula.getValues()["latex"];
//         std::string explanation = formula.getValues()["explanation"];
//         std::string unitOverride = "";
//         if (formula.getValues().find("unitOverride") != formula.getValues().end()) {
//           unitOverride = formula.getValues().find("unitOverride")->second;
//         }
//         std::string hideAnswer = "0";
//         if (formula.getValues().find("hideAnswer") != formula.getValues().end()) {
//           hideAnswer = formulas.getValues().find("hideAnswer")->second;
//         }
//         std::string isAnswer = formula.getValues()["isAnswer"];
//         std::string isIntermediate = formula.getValues()["isIntermediate"];
//         std::string result = formula.getValues()["result"];
//         Formula* f = assignment.addFormula(t);
//         f->latex = QString{latex.c_str()};
//         f->result = QString{result.c_str()};
//         f->explanation = QString{explanation.c_str()};
//         f->unitOverride = QString{unitOverride.c_str()};
//         if (isAnswer == "1") {
//           f->isAnswer = true;
//         } else {
//           f->isAnswer = false;
//         }
//
//         if (hideAnswer == "1") {
//           f->hideAnswer = true;
//         } else {
//           f->hideAnswer = false;
//         }
//
//         if (isIntermediate == "1") {
//           f->isIntermediate = true;
//         } else {
//           f->isIntermediate = false;
//         }
//       }
//
//       auto images = task.getChildren().find("images")->second;
//
//       std::vector<std::pair<int, XDFNode>> orderedImages;
//       for (auto& [n, image] : images.getChildren()) {
//         int order = std::stoi(image.getValues()["order"]);
//         orderedImages.push_back({order, image});
//       }
//       std::sort(orderedImages.begin(), orderedImages.end(), [](auto& a, auto& b){ return a.first < b.first; });
//
//       for (auto& [_, image] : orderedImages) {
//         std::string path = image.getValues()["path"];
//         std::string caption = image.getValues()["caption"];
//         Image* i = assignment.addImage(t, QString{path.c_str()});
//         i->caption = QString{caption.c_str()};
//       }
//     }
//   }
//
//   return assignment;
// }

