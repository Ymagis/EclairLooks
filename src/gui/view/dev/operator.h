#pragma once

#include <QtWidgets/QTabWidget>


class ImageOperator;

class OperatorWidget : public QTabWidget
{
  public:
    OperatorWidget(ImageOperator *op, QWidget *parent = nullptr);

  private:
    void setupUi();

  private:
    ImageOperator *m_operator;
};