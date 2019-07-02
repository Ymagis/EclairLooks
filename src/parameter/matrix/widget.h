#pragma once

#include <vector>

#include <core/types.h>
#include "../parameterwidget.h"


class ParameterMatrixWidget : public ParameterWidget
{
  public:
    ParameterMatrixWidget(Parameter *param, QWidget *parent = nullptr);

  public:
    void updateWidget(const Parameter &p) override;

  private:
    Matrix4x4 getValues();
    void setValues(const Matrix4x4 &m);

    std::vector<double> extractDoubles(const QString &str);

  private:
    MatrixParameter *m_matrixParam;

    QGridLayout *m_gridLayout;
    QDoubleValidator *m_validator;
};
