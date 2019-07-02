#pragma once

#include <string>

#include "../parameter.h"


class TextParameter : public Parameter
{
  public:
    TextParameter() = default;
    TextParameter(const std::string &name);

  public:
    std::string value() const;
    void setValue(const std::string &v);

    std::string defaultValue() const;
    void setDefaultValue(const std::string &v);

  public:
    ParameterWidget *newWidget(QWidget * parent = nullptr) override;

    void load(const QSettings *setting) override;
    void save(QSettings *setting) const override;

  private:
    std::string m_value;
    std::string m_default_value;
};