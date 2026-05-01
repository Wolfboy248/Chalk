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
#include "documentModel.hpp"
#include "historyDock.hpp"

static inline QString changeTypeToStr(ChangeType type) {
  switch (type) {
    case ChangeType::Structure: return "Structure"; break;
    case ChangeType::Content: return "Content"; break;
    case ChangeType::Metadata: return "Metadata"; break;
    case ChangeType::Selection: return "Selection"; break;
    default: return "Unknown"; break;
  }
}

class Editor : public QMainWindow {
  Q_OBJECT
private:
  void logChange(ChangeType t);

public:
  explicit Editor(QWidget* parent = nullptr);

  DocumentModel* doc() const { return mDoc; }

protected:
  void closeEvent(QCloseEvent* event) override;

private slots:
  void onChanged(ChangeType type);
  void onSaveStateChanged(bool unsaved);
  // void onFileChanged(const QString& path);
  void onTaskSelected(Task* task);

private:
  // Setup/window
  void setupMenu();
  void setupToolbar();
  void setupCentralWidget();
  void setupDocks();
  void updateWindowTitle();

  // Save/load
  void newAssignment();
  void save();
  void saveAs();
  void load();
  void exportToPdf();
  void openNameDialog();

private:
  DocumentModel* mDoc;

  PagesBridge* mPagesBridge;

  // Widgets
  QWebEngineView* mPagesContainer;
  MathInputDock* mMathDock;
  NavigatorWidget* mNavigator;
  HistoryDock* mHistoryDock;

  Task* mSelectedTask = nullptr;

  // QWidget* scrollContainer;
  // QVBoxLayout* scrollLayout;
  // QScrollArea* scrollArea;
  // QTextEdit* docEdit;
  // QListWidget* taskNavigator;
  // QWebEngineView* pagesContainer;
  // PagesBridge* pagesBridge;
  //
  // CommandManager* commandManager;
  //
  // MathInputDock* mathDock;
  // NavigatorWidget* navigator;
  //
  // Assignment assignment;
  //
  // QString currentFile = "";
  //
  // Task* selectedTask = nullptr;
  //
  // bool mUnsaved = true;
  //
  // std::vector<PageWidget*> pages;
};

