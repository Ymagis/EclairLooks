#pragma once

#include <QtWidgets/QPlainTextEdit>


class LogWidget : public QPlainTextEdit
{
  public:
    LogWidget(QWidget *parent = nullptr);
    ~LogWidget();
};
