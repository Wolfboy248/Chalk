#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QObject>
#include <QFileSystemWatcher>

#include <QTextEdit>

#include <editor.hpp>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  Editor editor{};
  editor.showMaximized();
  
  return app.exec();
}
