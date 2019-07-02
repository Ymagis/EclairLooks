#pragma once

#include <QtWidgets/QtWidgets>

#include <utils/generic.h>
#include "parameter.h"


class QVBoxLayout;

class ParameterWidget : public QWidget
{
  public:
    ParameterWidget(Parameter *param, QWidget *parent = nullptr);

  public:
    virtual void updateWidget(const Parameter &p) = 0;

  protected:
    Parameter *m_param;
    QVBoxLayout *m_layout;
};
