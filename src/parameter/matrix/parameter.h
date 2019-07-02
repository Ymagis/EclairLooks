#pragma once

#include <string>
#include <array>

#include <core/types.h>
#include "../parameter.h"


class MatrixParameter : public Parameter
{
  public:
    MatrixParameter() = default;
    MatrixParameter(const std::string &name);

  public:
    Matrix4x4 value() const;
    void setValue(const Matrix4x4 &v);

    Matrix4x4 defaultValue() const;
    void setDefaultValue(const Matrix4x4 &v);

  public:
    ParameterWidget *newWidget(QWidget * parent = nullptr) override;

    void load(const QSettings *setting) override;
    void save(QSettings *setting) const override;

  private:
    Matrix4x4 m_value;
    Matrix4x4 m_default_value = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
};