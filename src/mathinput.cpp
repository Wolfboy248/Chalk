#include <mathinput.hpp>

#include <QWebChannel>
#include <QWebEngineView>

MathInputDock::MathInputDock(QWidget* parent) : QDockWidget("Math Input", parent) {
  QWebEngineView* view = new QWebEngineView(this);
  setWidget(view);
  setMinimumWidth(400);
  setMaximumWidth(600);

  bridge = new Bridge();
  QWebChannel* channel = new QWebChannel();
  channel->registerObject("bridge", bridge);
  view->page()->setWebChannel(channel);

  view->setUrl(QUrl("qrc:/web/index.html"));

  // connect(bridge, &Bridge::updateFormula, this, [&]() {
  //
  // });
}

void MathInputDock::setTask(Task* task) {
  bridge->setTask(task);
  // qDebug() << "OMG SET TASK?! Task: " << task->title;
}

