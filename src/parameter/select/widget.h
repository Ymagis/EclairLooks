#pragma once

#include "../parameterwidget.h"


class ParameterSelectWidget : public ParameterWidget
{
  public:
    ParameterSelectWidget(Parameter *param, QWidget *parent = nullptr);

  public:
    void updateWidget(const Parameter &p) override;

  private:
    SelectParameter *m_selectParam;
    QComboBox *m_comboBox;
};
