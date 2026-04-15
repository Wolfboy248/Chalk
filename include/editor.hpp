#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QListWidget>
#include <qwebengineview.h>
#include <types.hpp>
#include <QObject>
#include <QVBoxLayout>
#include <QScrollArea>
#include "mathinput.hpp"
#include "navigator.hpp"
#include "page.hpp"
#include "pagesBridge.hpp"

class Editor : public QMainWindow {
  Q_OBJECT
public:
  Editor(QWidget* parent = nullptr);

public slots:
  void exportToPdf();

private slots:
  void onTaskSelected(Task* task);

private:
  void setupMenu();
  void setupToolbar();
  void setupCentralWidget();
  void setupDocks();

  void newAssignment();
  void save();
  void saveAs();
  void load();
  void test();

  QWidget* scrollContainer;
  QVBoxLayout* scrollLayout;
  QScrollArea* scrollArea;
  QTextEdit* docEdit;
  QListWidget* taskNavigator;
  QWebEngineView* pagesContainer;
  PagesBridge* pagesBridge;

  MathInputDock* mathDock;
  NavigatorWidget* navigator;

  Assignment assignment;

  QString currentFile = "";

  Task* selectedTask = nullptr;

  std::vector<PageWidget*> pages;
};

