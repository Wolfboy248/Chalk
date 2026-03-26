#include <bridge.hpp>
#include <QDebug>

void Bridge::receiveLatex(const QString& latex) {
  qDebug() << "Latex: " << latex;
}

