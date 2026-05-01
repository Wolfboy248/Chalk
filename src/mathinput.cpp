#include <filesystem>
#include <mathinput.hpp>

#include <QWebChannel>
#include <QWebEngineView>
#include "bridge.hpp"

MathInputDock::MathInputDock(QWidget* parent, Editor* editor) : QDockWidget("Math Input", parent), e{editor} {
  setAllowedAreas(Qt::AllDockWidgetAreas);
  setFeatures(
    QDockWidget::DockWidgetMovable |
    QDockWidget::DockWidgetFloatable
  );

  QWebEngineView* view = new QWebEngineView(this);
  setWidget(view);
  setMinimumWidth(400);
  setMaximumWidth(800);

  bridge = new Bridge(this, e);
  QWebChannel* channel = new QWebChannel();
  channel->registerObject("bridge", bridge);
  view->page()->setWebChannel(channel);

  view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

#ifdef NDEBUG
  view->setUrl(QUrl("qrc:/web/index.html"));
#else
  view->setUrl(QUrl::fromLocalFile(
    QString(
      (std::filesystem::current_path() / "web/index.html").string().c_str()
    )
  ));
#endif

  connect(view, &QWebEngineView::loadFinished, bridge, [&]() {
    bridge->setTask(lastTask);
  });

  connect(bridge, &Bridge::taskChanged, this, [&]() {
    qDebug() << "taskChanged, didnt emit changed";
    // emit changed();
  });
  connect(bridge, &Bridge::evaluateTask, this, [&]() {
    qDebug() << "evaluateTask, didnt emit changed";
    // emit changed();
  });
  connect(bridge, &Bridge::resultsReady, this, [&]() {
    qDebug() << "resultsReady, didnt emit changed";
    // emit changed();
  });
  connect(bridge, &Bridge::updatedExplanation, this, [&]() {
    qDebug() << "updatedExplanation";
    emit changed();
  });

  // connect(bridge, &Bridge::updateFormula, this, [&]() {
  //
  // });
}

// void MathInputDock::setAssignment(Assignment* a) {
//   bridge->setAssignment(a);
//   assignment = a;
// }

void MathInputDock::setTask(Task* task) {
  bridge->setTask(task);
  lastTask = task;
  // qDebug() << "OMG SET TASK?! Task: " << task->title;
}

