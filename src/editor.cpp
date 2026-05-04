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

#include <QMessageBox>

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

#include <QStatusBar>

#define WINDOW_TITLE_PREFIX "Chalk - "

void logAssignment(const Assignment& a) {
  qDebug() << "Assignment: " << a.title;
  qDebug() << "Tasks:";
  for (int i = 0; i < a.tasks.size(); i++) {
    const auto& task = a.tasks[i];
    qDebug() << task->id << ". " << task->title;
    
    for (int j = 0; j < task->formulas.size(); j++) {
      const auto& formula = task->formulas[j];
      qDebug() << "  |- " << j << ". " << formula->latex;
    }
  }
}

void Editor::logChange(ChangeType type) {
  QString changeStr = changeTypeToStr(type);
  qDebug() << "Detected change of type: " + changeStr;
}

Editor::Editor(QWidget* parent) : QMainWindow(parent) {
  mDoc = new DocumentModel(this);

  setupMenu();
  setupToolbar();
  setupCentralWidget();
  setupDocks();

  // Model wired to the ui. NOWHERE ELSE SHOULD UPDATE LOGIC BE. ONLY HERE
  connect(mDoc, &DocumentModel::changed, this, &Editor::onChanged);
  connect(
    mDoc,
    &DocumentModel::saveStateChanged,
    this,
    &Editor::onSaveStateChanged
  );
  connect(mDoc, &DocumentModel::fileChanged, this, [this](const QString&) {
    updateWindowTitle();
  });

  newAssignment();
}

void Editor::closeEvent(QCloseEvent* event) {
  if (mDoc->isUnsaved()) {
    auto result = QMessageBox::warning(
      this,
      "Unsaved Changes",
      "You have unsaved changes. Save?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );

    if (result == QMessageBox::Save) {
      save();
      event->accept();
    } else if (result == QMessageBox::Cancel) {
      event->ignore();
    } else {
      event->accept();
    }
  } else {
    event->accept();
  }
}

// Supa importante funktion. All changes should go through here.
void Editor::onChanged(ChangeType type) {
  qDebug() << "On change";
  logChange(type);
  logAssignment(mDoc->data());
  mHistoryDock->refresh();
  switch (type) {
    // Full re-render
    case ChangeType::Structure:
      mPagesBridge->update();
      mNavigator->refresh();
      // mMathDock->refresh();
      break;

    case ChangeType::Content:
      mPagesBridge->update(); // Soft re-render
      mMathDock->refresh();
      break;

    case ChangeType::Metadata:
      mPagesBridge->updateFull(); // Names/title
      updateWindowTitle();
      // navigator doesnt use names or title
      break;

    // No rendering will be done.
    case ChangeType::Selection:
      mMathDock->refresh();
      break;
  }
}

void Editor::onSaveStateChanged(bool unsaved) {
  updateWindowTitle();
}

void Editor::onTaskSelected(int id) {
  if (id == -1) {
    mMathDock->setTask(-1);
    mDoc->selectedTaskChanged();
  }
  // mSelectedTask = task;
  mMathDock->setTask(id);
  mPagesBridge->scrollToTask(mDoc->data().getTask(id));
  mDoc->selectedTaskChanged();
}

// === Setup/window ===
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
  // fileMenu->addAction("Update document", this, &Editor::test);
  fileMenu->addSeparator();
  fileMenu->addAction("Exit", qApp, &QApplication::quit);

  QMenu* editMenu = menuBar()->addMenu("&Edit");

  QAction* undoAction = new QAction("Undo", this);
  undoAction->setShortcut(QKeySequence::Undo);
  connect(undoAction, &QAction::triggered, mDoc, &DocumentModel::undo);
  editMenu->addAction(undoAction);

  QAction* redoAction = new QAction("Redo", this);
  redoAction->setShortcut(QKeySequence::Redo);
  connect(redoAction, &QAction::triggered, mDoc, &DocumentModel::redo);
  editMenu->addAction(redoAction);

  editMenu->addSeparator();
  editMenu->addAction("Names...", this, &Editor::openNameDialog);
}

void Editor::setupToolbar() {
  // QToolBar* toolbar = new QToolBar("Text Formatting");
  // addToolBar(toolbar);
  //
  // QAction* boldAction = toolbar->addAction("B");
  // boldAction->setCheckable(true);
  //
  // connect(boldAction, &QAction::toggled, [&](bool checked) {
  //   // Blah blah balh
  // });
}

void Editor::setupCentralWidget() {
  QTabWidget* tabs = new QTabWidget(this);

  mPagesContainer = new QWebEngineView(this);
  QWebEngineView* container = mPagesContainer;
  mPagesBridge = new PagesBridge(this, this);
  PagesBridge* bridge = mPagesBridge;
  QWebChannel* channel = new QWebChannel();
  channel->registerObject("bridge", bridge);
  container->page()->setWebChannel(channel);
  // bridge->setAssignment(&assignment);

#ifdef NDEBUG
  container->setUrl(QUrl("qrc:/web/pages.html"));
#else
  container->setUrl(QUrl::fromLocalFile(
    QString(
      (std::filesystem::current_path() / "web/pages.html").string().c_str()
    )
  ));
#endif

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
  setDockOptions(
    QMainWindow::AllowTabbedDocks |
    QMainWindow::AllowNestedDocks |
    QMainWindow::AnimatedDocks
  );

  mNavigator = new NavigatorWidget(this, this);
  mHistoryDock = new HistoryDock(this, this);

  addDockWidget(Qt::RightDockWidgetArea, mNavigator);
  addDockWidget(Qt::RightDockWidgetArea, mHistoryDock);

  tabifyDockWidget(mNavigator, mHistoryDock);
  mNavigator->raise();

  mMathDock = new MathInputDock(this, this);
  // mMathDock->setAssignment(&assignment);
  addDockWidget(Qt::LeftDockWidgetArea, mMathDock);
  resizeDocks({mMathDock, mHistoryDock}, {500, 300}, Qt::Horizontal);

  connect(
    mNavigator,
    &NavigatorWidget::taskSelected,
    this,
    &Editor::onTaskSelected
  );
  //
  // connect(
  //   navigator,
  //   &NavigatorWidget::changed,
  //   [&]() {
  //     if (assignment.tasks.size() == 0) {
  //       mathDock->setTask(nullptr);
  //     }
  //   }
  // );
  //
  // connect(
  //   pagesBridge,
  //   &PagesBridge::updatedTaskTitle,
  //   this,
  //   [&]() {
  //     qDebug() << "Update to document from: pagesBridge updatedTaskTitle";
  //     updateToDocument();
  //     // navigator->refresh();
  //   }
  // );
  //
  // connect(
  //   mathDock,
  //   &MathInputDock::changed,
  //   [&]() {
  //     qDebug() << "Update to document from: mathinput changed";
  //     updateToDocument();
  //     // pagesBridge->update();
  //   }
  // );
}

void Editor::updateWindowTitle() {
  QString winTitle = WINDOW_TITLE_PREFIX + mDoc->data().title;

  if (mDoc->isUnsaved()) {
    winTitle += "*";
  }

  setWindowTitle(winTitle);
}

// === Save/load ===
void Editor::newAssignment() {
  mDoc->reset();
}

void Editor::save() {
  if (!mDoc->save()) {
    saveAs();
    return;
  }
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

  if (mDoc->saveAs(fileName)) {
    statusBar()->showMessage("Document saved", 2000);
  }
}

void Editor::load() {
  QString fileName = QFileDialog::getOpenFileName(
    this,
    "Open File",
    "",
    "BMF Files (*.bmf)"
  );

  if (fileName == "") return;

  mDoc->load(fileName);
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

    mPagesContainer->page()->printToPdf(fileName);
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

  for (const auto& s : mDoc->data().names) {
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
    std::vector<QString> newNames{};
    for (int i = 0; i < list->count(); i++) {
      newNames.push_back(list->item(i)->text());
    }
    mDoc->updateNames(newNames);
  });

  dialog->adjustSize();
  dialog->move(this->geometry().center() - dialog->rect().center());

  dialog->exec();
}

void Editor::refreshMathDockTask() {
}

