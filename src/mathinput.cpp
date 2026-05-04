#include <filesystem>
#include <mathinput.hpp>

#include <QEvent>
#include <QApplication>

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
    bridge->setTask(lastTaskId);
  });

  connect(qApp, &QApplication::focusChanged, this, [this](QWidget* old, QWidget* now) {
    if (old && this->isAncestorOf(old) && (!now || !this->isAncestorOf(now))) {
      bridge->lostWindowFocus(); 
    }
  });
}

bool MathInputDock::eventFilter(QObject* watched, QEvent* event) {
  qDebug() << "OMG!!!";
  if (event->type() == QEvent::FocusOut) {
    qDebug() << "FOCUSOUT!!";
    bridge->lostWindowFocus();
  }
  return QDockWidget::eventFilter(watched, event);
}

void MathInputDock::refresh() {
  bridge->refresh();
  // bridge->taskHasChanged();
}

void MathInputDock::setTask(int id) {
  bridge->setTask(id);
  lastTaskId = id;
}

