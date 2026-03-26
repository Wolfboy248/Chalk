#include "mathinput.hpp"
#include "navigator.hpp"
#include "page.hpp"
#include <editor.hpp>

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

void Editor::setupMenu() {
  QMenu* fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction("New");
  fileMenu->addAction("Open...");
  fileMenu->addAction("Save");
  fileMenu->addSeparator();
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
  QWidget* docContainer = new QWidget(this);
  QVBoxLayout* docLayout = new QVBoxLayout(docContainer);
  docLayout->setSpacing(20);
  docLayout->setAlignment(Qt::AlignHCenter);

  scrollArea = new QScrollArea(this);
  scrollArea->setWidget(docContainer);
  scrollArea->setWidgetResizable(true);
  scrollArea->setAlignment(Qt::AlignHCenter);
  setCentralWidget(scrollArea);

  PageWidget* firstPage = new PageWidget(docContainer);
  pages.push_back(firstPage);
  docLayout->addWidget(firstPage);

  firstPage->layout->setSpacing(20);

  QLineEdit* titleEdit = new QLineEdit(firstPage);
  titleEdit->setPlaceholderText("Assignment Title");
  titleEdit->setText(assignment.title);
  titleEdit->setStyleSheet("font-size: 24px; font-weight: bold; color: #000;");
  firstPage->layout->addWidget(titleEdit);

  connect(titleEdit, &QLineEdit::textChanged, [&](const QString& text) {
    assignment.title = text;
  });

  // for (int i = 0; i < 2; i++) {
  //   PageWidget* page = new PageWidget(docContainer);
  //   pages.push_back(page);
  // }
  //
  // for (auto page : pages) {
  //   docLayout->addWidget(page);
  // }
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
  if (pages.empty()) return;

  QPrinter printer{QPrinter::HighResolution};
  printer.setPageSize(QPageSize::A4);
  printer.setPageOrientation(QPageLayout::Portrait);
  printer.setFullPage(true);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName("assignment.pdf");

  QPainter painter{&printer};

  QRect deviceRect = painter.viewport();
  double scale = deviceRect.width() / 595.0;

  painter.scale(scale, scale);

  int margin = 40;
  int y = margin;

  QFont titleFont("Arial", -1, QFont::Bold);
  titleFont.setPixelSize(24);
  QRect titleRect{margin, margin, 595 - margin * 2, 100};
  painter.setFont(titleFont);
  painter.setPen(QColor(0, 0, 0));
  painter.drawText(titleRect, Qt::AlignLeft | Qt::TextWordWrap, "Testing!!!");

  painter.setPen(QPen(Qt::black, 2));
  painter.drawRect(QRect(1, 1, 593, 840));

  painter.end();
}

