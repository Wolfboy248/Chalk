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

  void undo();
  void redo();

  void setAssignment(Assignment assignment);

  CommandManager* cmdMgr() { return commandManager; }

public slots:
  void exportToPdf();

  void updateToDocument(bool softUpdate = true);

private slots:
  void onTaskSelected(Task* task);

private:
  void setupMenu();
  void setupToolbar();
  void setupCentralWidget();
  void setupDocks();

  void updateWindowTitle(bool somethingChanged = false);

  void newAssignment();
  void save();
  void saveAs();
  void load();
  void test();

  void openNameDialog();

protected:
  void closeEvent(QCloseEvent* event) override;

  QWidget* scrollContainer;
  QVBoxLayout* scrollLayout;
  QScrollArea* scrollArea;
  QTextEdit* docEdit;
  QListWidget* taskNavigator;
  QWebEngineView* pagesContainer;
  PagesBridge* pagesBridge;

  CommandManager* commandManager;

  MathInputDock* mathDock;
  NavigatorWidget* navigator;

  Assignment assignment;

  QString currentFile = "";

  Task* selectedTask = nullptr;

  bool mUnsaved = true;

  std::vector<PageWidget*> pages;
};

