#pragma once

#include "../parameterwidget.h"


class ParameterTextWidget : public ParameterWidget
{
  public:
    ParameterTextWidget(Parameter *param, QWidget *parent = nullptr);

  public:
    void updateWidget(const Parameter &p) override;

  private:
    TextParameter *m_textParam;
    QTextEdit *m_textEdit;
};