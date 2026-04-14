#include "mathinput.hpp"
#include "navigator.hpp"
#include "page.hpp"
#include "pagesBridge.hpp"
#include <filesystem>
#include <pages.hpp>
#include <editor.hpp>

#include <assignmentRepository.hpp>

#include <QTimer>

#include <QApplication>
#include <QToolBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QDockWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include <QFileSystemWatcher>
#include <bridge.hpp>

#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

#include <QLineEdit>

#include <QDebug>

void logAssignment(const Assignment& a) {
  qDebug() << "Assignment: " << a.title;
  qDebug() << "Tasks:";
  for (int i = 0; i < a.tasks.size(); i++) {
    const auto& task = a.tasks[i];
    qDebug() << i << ". " << task->title;
    
    for (int j = 0; j < task->formulas.size(); j++) {
      const auto& formula = task->formulas[j];
      qDebug() << " |- " << j << ". " << formula->latex;
    }
  }
}

Editor::Editor(QWidget* parent) : QMainWindow(parent) {
  setupMenu();
  setupToolbar();
  setupCentralWidget();
  setupDocks();

  // exportToPdf();
}

void Editor::test() {
  pagesBridge->update();
}

void Editor::save() {
  QString fileName = QFileDialog::getSaveFileName(
    this,
    "Save File",
    "",
    "BMF Files (*.bmf)"
  );

  if (fileName == "") return;

  if (!fileName.endsWith(".bmf")) {
    fileName += ".bmf";
  }

  AssignmentRepository::save(assignment, fileName);
}

void Editor::load() {
  QString fileName = QFileDialog::getOpenFileName(
    this,
    "Open File",
    "",
    "BMF Files (*.bmf)"
  );

  if (fileName == "") return;

  assignment = AssignmentRepository::load(fileName);
  qDebug() << assignment.title;
  pagesBridge->setAssignment(&assignment);
  navigator->setAssignment(&assignment);
  mathDock->setAssignment(&assignment);
  pagesBridge->update();
}

void Editor::setupMenu() {
  QMenu* fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction("New");
  fileMenu->addAction("Open...", this, &Editor::load);
  fileMenu->addAction("Save", this, &Editor::save);
  fileMenu->addSeparator();
  fileMenu->addAction("Update", this, &Editor::test);
  fileMenu->addAction("Export", this, &Editor::exportToPdf);
  fileMenu->addSeparator();
  fileMenu->addAction("Exit", qApp, &QApplication::quit);

  QMenu* editMenu = menuBar()->addMenu("&Edit");
  editMenu->addAction("Undo");
  editMenu->addAction("Redo");
}

void Editor::setupToolbar() {
  QToolBar* toolbar = new QToolBar("Text Formatting");
  addToolBar(toolbar);

  QAction* boldAction = toolbar->addAction("B");
  boldAction->setCheckable(true);

  connect(boldAction, &QAction::toggled, [&](bool checked) {
    // Blah blah balh
  });
}

void Editor::setupCentralWidget() {
  pagesContainer = new QWebEngineView(this);
  QWebEngineView* container = pagesContainer;
  pagesBridge = new PagesBridge(this);
  PagesBridge* bridge = pagesBridge;
  QWebChannel* channel = new QWebChannel();
  channel->registerObject("bridge", bridge);
  container->page()->setWebChannel(channel);
  bridge->setAssignment(&assignment);
  setCentralWidget(container);
  // container->setUrl(QUrl("qrc:/web/pages.html"));
  container->setUrl(QUrl::fromLocalFile(
    QString(
      (std::filesystem::current_path() / "web/pages.html").string().c_str()
    )
  ));
  connect(container, &QWebEngineView::loadFinished, this, [bridge, this]() {
    bridge->setBg(
      QWidget::palette().color(QWidget::backgroundRole()).name()
    );
  });
}

void Editor::setupDocks() {
  navigator = new NavigatorWidget(this);
  navigator->setAssignment(&assignment);
  navigator->setMinimumWidth(400);
  navigator->setMaximumWidth(600);
  connect(navigator, &NavigatorWidget::changed, [&]() {
    // onTaskSelected(selectedTask);
    // qDebug() << "Current index: " 
    logAssignment(assignment);
    pagesBridge->update();
  });
  addDockWidget(Qt::RightDockWidgetArea, navigator);

  mathDock = new MathInputDock(this);
  mathDock->setAssignment(&assignment);
  addDockWidget(Qt::LeftDockWidgetArea, mathDock);

  connect(
    navigator,
    &NavigatorWidget::taskSelected,
    this,
    &Editor::onTaskSelected
  );

  connect(
    pagesBridge,
    &PagesBridge::updatedTaskTitle,
    this,
    [&]() {
      navigator->refresh();
    }
  );

  connect(
    mathDock,
    &MathInputDock::changed,
    [&]() {
      pagesBridge->update();
    }
  );
}

void Editor::onTaskSelected(Task* task) {
  selectedTask = task;
  mathDock->setTask(task);
  if (!task) return;
}

void Editor::exportToPdf() {
  pagesContainer->page()->runJavaScript(R"(
    document.body.offsetHeight;
  )", [this](const QVariant&) {
    QTimer::singleShot(100, this, [this]() {
      QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export to PDF",
        "",
        "PDF (*.pdf)"
      );

      if (fileName == "") return;

      if (!fileName.endsWith(".pdf")) {
        fileName += ".pdf";
      }

      pagesContainer->page()->printToPdf(fileName);
    });
  });
}

