#pragma once

#include <QWidget>
#include <QVBoxLayout>

class PageWidget : public QWidget {
public:
  PageWidget(QWidget* parent = nullptr) : QWidget(parent) {
    setFixedSize(595, 842);
    setStyleSheet(R"(
      background-color: white;
      border: 1px solid #999999;
      border-radius: 3px;
    )");

    layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);
  }

  QVBoxLayout* layout;
};

