#pragma once

#include <QWidget>
#include <QDockWidget>

class GraphWidget : public QWidget {
public:
  explicit GraphWidget(QWidget* parent = nullptr);

protected:
  void paintEvent(QPaintEvent* event) override;
};

class GraphDockWidget : public QDockWidget {
  Q_OBJECT
public:
  GraphDockWidget(QWidget* parent = nullptr) : QDockWidget("Graph", parent) {
    graphWidget = new GraphWidget(this);
    // graphWidget->resize(400, 600);
    setWidget(graphWidget);

    setAllowedAreas(Qt::AllDockWidgetAreas);
    setFeatures(
      QDockWidget::DockWidgetMovable |
      QDockWidget::DockWidgetFloatable
    );
  }

private:
  GraphWidget* graphWidget;
};

