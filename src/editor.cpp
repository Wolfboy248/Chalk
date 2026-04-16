#include "mathinput.hpp"
#include "navigator.hpp"
#include "page.hpp"
#include "pagesBridge.hpp"
#include <filesystem>
#include <pages.hpp>
#include <editor.hpp>

#include <assignmentRepository.hpp>

#include <QTimer>
#include <QLabel>
#include <QPushButton>

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

#include <graph.hpp>

#include <QLineEdit>

#include <QDebug>

#define WINDOW_TITLE_PREFIX "Chalk - "

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
  setDockOptions(
    QMainWindow::AllowTabbedDocks |
    QMainWindow::AllowNestedDocks |
    QMainWindow::AnimatedDocks
  );
  setupDocks();
  setWindowTitle(WINDOW_TITLE_PREFIX + QString("unsaved*"));

  // exportToPdf();
}

void Editor::test() {
  pagesBridge->update();
}

void Editor::save() {
  if (currentFile == "") {
    saveAs();
    return;
  }

  AssignmentRepository::save(assignment, currentFile);
  setWindowTitle(WINDOW_TITLE_PREFIX + currentFile + " - " + assignment.title);
}

void Editor::openNameDialog() {
  QDialog* dialog = new QDialog(this);
  dialog->setWindowTitle("Assignment names");
  dialog->setWindowFlags(Qt::Dialog);
  dialog->setWindowModality(Qt::ApplicationModal);

  dialog->setMinimumSize(400, 300);

  QVBoxLayout* layout = new QVBoxLayout(dialog);

  QListWidget* list = new QListWidget(dialog);
  list->setEditTriggers(
    QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked
  );
  layout->addWidget(list);

  auto* addBtn = new QPushButton("Add", dialog);
  auto* removeBtn = new QPushButton("Remove", dialog);

  auto* btnLayout = new QHBoxLayout();
  btnLayout->addWidget(addBtn);
  btnLayout->addWidget(removeBtn);

  layout->addLayout(btnLayout);

  for (const auto& s : assignment.names) {
    list->addItem(s);
  }

  connect(addBtn, &QPushButton::clicked, dialog, [&]() {
    auto* item = new QListWidgetItem("Anders Andersen");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    list->addItem(item);
    list->setCurrentItem(item);
    list->editItem(item);
  });
  connect(removeBtn, &QPushButton::clicked, dialog, [&]() {
    delete list->takeItem(list->currentRow());
  });

  connect(dialog, &QDialog::finished, this, [&](int result) {
    // if (result != QDialog::Accepted) return;
    assignment.names.clear();
    for (int i = 0; i < list->count(); i++) {
      assignment.names.push_back(list->item(i)->text());
    }

    pagesBridge->updateFull();
  });

  dialog->adjustSize();
  dialog->move(this->geometry().center() - dialog->rect().center());

  dialog->exec();
}

void Editor::saveAs() {
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
  currentFile = fileName;
  setWindowTitle(WINDOW_TITLE_PREFIX + currentFile + " - " + assignment.title);
}

void Editor::newAssignment() {
  Assignment ass{};
  assignment = std::move(ass);

  setWindowTitle(WINDOW_TITLE_PREFIX + QString("unsaved*"));
  pagesBridge->setAssignment(&assignment);
  navigator->setAssignment(&assignment);
  mathDock->setAssignment(&assignment);
  pagesBridge->update();

  currentFile = "";
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

  currentFile = fileName;
  setWindowTitle(WINDOW_TITLE_PREFIX + currentFile + " - " + assignment.title);
}

void Editor::setupMenu() {
  QMenu* fileMenu = menuBar()->addMenu("&File");

  QAction* newAction = new QAction("New", this);
  newAction->setShortcut(QKeySequence::New);
  connect(newAction, &QAction::triggered, this, &Editor::newAssignment);

  QAction* openAction = new QAction("Open...", this);
  openAction->setShortcut(QKeySequence::Open);
  connect(openAction, &QAction::triggered, this, &Editor::load);

  fileMenu->addAction(newAction);
  fileMenu->addAction(openAction);

  QAction* saveAction = new QAction("Save", this);
  saveAction->setShortcut(QKeySequence::Save);
  connect(saveAction, &QAction::triggered, this, &Editor::save);

  QAction* saveAsAction = new QAction("Save as", this);
  saveAsAction->setShortcut(QKeySequence::SaveAs);
  connect(saveAsAction, &QAction::triggered, this, &Editor::saveAs);
  fileMenu->addAction(saveAction);
  fileMenu->addAction(saveAsAction);

  // fileMenu->addAction("Save as", )
  fileMenu->addSeparator();

  QAction* exportAction = new QAction("Export...", this);
  exportAction->setShortcut(QKeySequence::Print);
  connect(exportAction, &QAction::triggered, this, &Editor::exportToPdf);

  fileMenu->addAction(exportAction);
  fileMenu->addAction("Update document", this, &Editor::test);
  fileMenu->addSeparator();
  fileMenu->addAction("Exit", qApp, &QApplication::quit);

  QMenu* editMenu = menuBar()->addMenu("&Edit");
  editMenu->addAction("Undo");
  editMenu->addAction("Redo");
  editMenu->addSeparator();
  editMenu->addAction("Names...", this, &Editor::openNameDialog);
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
  QTabWidget* tabs = new QTabWidget(this);

  pagesContainer = new QWebEngineView(this);
  QWebEngineView* container = pagesContainer;
  pagesBridge = new PagesBridge(this);
  PagesBridge* bridge = pagesBridge;
  QWebChannel* channel = new QWebChannel();
  channel->registerObject("bridge", bridge);
  container->page()->setWebChannel(channel);
  bridge->setAssignment(&assignment);
  // setCentralWidget(container);
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

  GraphDockWidget* graphWidget = new GraphDockWidget(this);
  addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, graphWidget);

  tabs->addTab(container, "Document");
  tabs->addTab(graphWidget, "Graph");

  setCentralWidget(tabs);
}

void Editor::setupDocks() {
  navigator = new NavigatorWidget(this);
  navigator->setAssignment(&assignment);
  navigator->setMinimumWidth(400);
  navigator->setMaximumWidth(600);
  connect(navigator, &NavigatorWidget::changed, [&]() {
    // onTaskSelected(selectedTask);
    // qDebug() << "Current index: " 
    // logAssignment(assignment);
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
    navigator,
    &NavigatorWidget::changed,
    [&]() {
      if (assignment.tasks.size() == 0) {
        mathDock->setTask(nullptr);
      }
    }
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
  QTimer::singleShot(100, this, [this]() {
    QString fileName = QFileDialog::getSaveFileName(
      this,
      "Export to PDF",
      "",
      "PDF (*.pdf)"
    );

    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf")) fileName += ".pdf";

    pagesContainer->page()->printToPdf(fileName);
  });
  // pagesContainer->page()->runJavaScript(R"(
  //   document.body.offsetHeight || 0;
  // )", [this](const QVariant&) {
  //   QTimer::singleShot(100, this, [this]() {
  //     QString fileName = QFileDialog::getSaveFileName(
  //       this,
  //       "Export to PDF",
  //       "",
  //       "PDF (*.pdf)"
  //     );
  //
  //     if (fileName == "") return;
  //
  //     if (!fileName.endsWith(".pdf")) {
  //       fileName += ".pdf";
  //     }
  //
  //     pagesContainer->page()->printToPdf(fileName);
  //   });
  // });
}

