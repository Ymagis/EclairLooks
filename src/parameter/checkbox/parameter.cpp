#include "parameter.h"
#include "widget.h"

#include <QtCore/QSettings>


CheckBoxParameter::CheckBoxParameter(const std::string &name, bool value)
    : Parameter(name), m_value(value), m_default_value(value)
{
}

bool CheckBoxParameter::value() const { return m_value; }

void CheckBoxParameter::setValue(const bool &v)
{
    m_value = v;
    EmitEvent<UpdateValue>(*this);
}

bool CheckBoxParameter::defaultValue() const { return m_default_value; }

void CheckBoxParameter::setDefaultValue(const bool &v)
{
    m_default_value = v;
    EmitEvent<UpdateSpecification>(*this);
}

ParameterWidget *CheckBoxParameter::newWidget(QWidget * parent)
{
    return new ParameterCheckBoxWidget(this, parent);
}

void CheckBoxParameter::load(const QSettings *setting)
{
    setValue(setting->value(QString::fromStdString(name())).toBool());
}

void CheckBoxParameter::save(QSettings *setting) const
{
    setting->setValue(QString::fromStdString(name()), value());
}