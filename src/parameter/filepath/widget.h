#pragma once

#include "../parameterwidget.h"


class ParameterFilePathWidget : public ParameterWidget
{
  public:
    ParameterFilePathWidget(Parameter *param, QWidget *parent = nullptr);

  public:
    void updateWidget(const Parameter &p) override;

  private:
    void UpdateLineEdit(const Parameter &p);

  private:
    FilePathParameter *m_filePathParam;
    QLineEdit *m_lineEdit;
    QToolButton *m_toolButton;
};
