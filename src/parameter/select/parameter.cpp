#include "parameter.h"
#include "widget.h"

#include <QtCore/QDebug>


SelectParameter::SelectParameter(const std::string &name) : Parameter(name) {}

SelectParameter::SelectParameter(const std::string &name,
                                 const std::vector<std::string> &choices)
    : Parameter(name), m_choices(choices)
{
    if (m_choices.size() > 0)
        m_value = m_default_value = m_choices[0];
}

SelectParameter::SelectParameter(const std::string &name,
                                 const std::vector<std::string> &choices,
                                 const std::string &default_value)
    : Parameter(name), m_value(default_value), m_default_value(default_value),
      m_choices(choices)
{
}

std::string SelectParameter::value() const { return m_value; }

void SelectParameter::setValue(const std::string &v)
{
    m_value = v;
    EmitEvent<UpdateValue>(*this);
}

std::string SelectParameter::defaultValue() const { return m_default_value; }

void SelectParameter::setDefaultValue(const std::string &v)
{
    m_default_value = v;
    EmitEvent<UpdateSpecification>(*this);
}

std::vector<std::string> SelectParameter::choices() const { return m_choices; }

std::vector<std::string> SelectParameter::tooltips() const { return m_tooltips; }

void SelectParameter::setChoices(const std::vector<std::string> &v,
                                 const std::vector<std::string> &t)
{
    if (t.size() > 0 && v.size() != t.size()) {
        qWarning() << "Choices / Tooltip size don't match !";
        return;
    }

    m_choices  = v;
    m_tooltips = t;
    EmitEvent<UpdateSpecification>(*this);
}

ParameterWidget *SelectParameter::newWidget(QWidget *parent)
{
    return new ParameterSelectWidget(this, parent);
}

void SelectParameter::load(const QSettings *setting)
{
    setValue(setting->value(QString::fromStdString(name())).toString().toStdString());
}

void SelectParameter::save(QSettings *setting) const
{
    setting->setValue(QString::fromStdString(name()), QString::fromStdString(value()));
}