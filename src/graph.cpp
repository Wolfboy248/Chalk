#include <graph.hpp>

#include <QPainter>

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent) {}

void GraphWidget::paintEvent(QPaintEvent*) {
  QPainter p{this};
  p.setRenderHint(QPainter::Antialiasing);

  int w = width();
  int h = height();

  int originX = w / 2;
  int originY = h / 2;

  p.drawLine(0, originY, w, originY);
  p.drawLine(originX, 0, originX, h);

  double scale = 50.0;
}

