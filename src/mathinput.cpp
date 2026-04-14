#include <filesystem>
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

  view->setUrl(QUrl::fromLocalFile(
    QString(
      (std::filesystem::current_path() / "web/index.html").string().c_str()
    )
  ));

  connect(bridge, &Bridge::taskChanged, this, [&]() {
    emit changed();
  });
  connect(bridge, &Bridge::evaluateTask, this, [&]() {
    emit changed();
  });
  connect(bridge, &Bridge::resultsReady, this, [&]() {
    emit changed();
  });
  connect(bridge, &Bridge::updatedExplanation, this, [&]() {
      emit changed();
  });

  // connect(bridge, &Bridge::updateFormula, this, [&]() {
  //
  // });
}

void MathInputDock::setTask(Task* task) {
  bridge->setTask(task);
  // qDebug() << "OMG SET TASK?! Task: " << task->title;
}

