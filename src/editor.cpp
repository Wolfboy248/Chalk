#include "mathinput.hpp"
#include "navigator.hpp"
#include "page.hpp"
#include "pagesBridge.hpp"
#include <pages.hpp>
#include <editor.hpp>

#include <QTimer>

#include <QApplication>
#include <QToolBar>
#include <QMenuBar>
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

void Editor::setupMenu() {
  QMenu* fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction("New");
  fileMenu->addAction("Open...");
  fileMenu->addAction("Save");
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
  container->setUrl(QUrl::fromLocalFile("/home/georgek/dev/c/bettermaths/web/pages.html"));
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
  });
  addDockWidget(Qt::RightDockWidgetArea, navigator);

  mathDock = new MathInputDock(this);
  mathDock->setAssignment(&assignment);
  addDockWidget(Qt::LeftDockWidgetArea, mathDock);

  connect(navigator, &NavigatorWidget::taskSelected, this, &Editor::onTaskSelected);
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
      pagesContainer->page()->printToPdf("output.pdf");
    });
  });
}

