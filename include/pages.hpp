#pragma once

#include <QDockWidget>
#include <QWebEngineView>

class PagesDock : public QDockWidget {
  Q_OBJECT
public:
  PagesDock(QWidget* parent = nullptr) : QDockWidget{parent} {
    QWebEngineView* view = new QWebEngineView(this);
    setWidget(view);
    setMinimumWidth(400);
    setMaximumWidth(600);

    // bridge = new Bridge();
    // QWebChannel* channel = new QWebChannel();
    // channel->registerObject("bridge", bridge);
    // view->page()->setWebChannel(channel);

    view->setUrl(QUrl("qrc:/web/pages.html"));
  }


};

