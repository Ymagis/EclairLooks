#pragma once

#include <string>

#include "../parameter.h"


class CheckBoxParameter : public Parameter
{
  public:
    CheckBoxParameter() = default;
    CheckBoxParameter(const std::string &name, bool value);

  public:
    bool value() const;
    void setValue(const bool &v);

    bool defaultValue() const;
    void setDefaultValue(const bool &v);

  public:
    ParameterWidget *newWidget(QWidget * parent = nullptr) override;

    void load(const QSettings *setting) override;
    void save(QSettings *setting) const override;

  private:
    bool m_value;
    bool m_default_value;
};