#pragma once

#include <string>

#include "../parameter.h"


class SelectParameter : public Parameter
{
  public:
    SelectParameter() = default;
    SelectParameter(const std::string &name);
    SelectParameter(const std::string &name, const std::vector<std::string> &choices);
    SelectParameter(const std::string &name, const std::vector<std::string> &choices,
                    const std::string &default_value);

  public:
    std::string value() const;
    void setValue(const std::string &v);

    std::string defaultValue() const;
    void setDefaultValue(const std::string &v);

    std::vector<std::string> choices() const;
    std::vector<std::string> tooltips() const;
    void setChoices(const std::vector<std::string> &v, const std::vector<std::string> &t = std::vector<std::string>());

  public:
    ParameterWidget *newWidget(QWidget * parent = nullptr) override;

    void load(const QSettings *setting) override;
    void save(QSettings *setting) const override;

  private:
    std::string m_value;
    std::string m_default_value;
    std::vector<std::string> m_choices;
    std::vector<std::string> m_tooltips;
};