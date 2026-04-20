#include <filesystem>
#include <mathinput.hpp>

#include <QWebChannel>
#include <QWebEngineView>

MathInputDock::MathInputDock(QWidget* parent) : QDockWidget("Math Input", parent) {
  setAllowedAreas(Qt::AllDockWidgetAreas);
  setFeatures(
    QDockWidget::DockWidgetMovable |
    QDockWidget::DockWidgetFloatable
  );

  QWebEngineView* view = new QWebEngineView(this);
  setWidget(view);
  setMinimumWidth(400);
  setMaximumWidth(800);

  bridge = new Bridge();
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
  lastTask = task;
  // qDebug() << "OMG SET TASK?! Task: " << task->title;
}

