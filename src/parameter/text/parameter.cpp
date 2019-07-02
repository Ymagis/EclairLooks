#include "parameter.h"
#include "widget.h"


TextParameter::TextParameter(const std::string &name) : Parameter(name)
{

}

std::string TextParameter::value() const { return m_value; }

void TextParameter::setValue(const std::string &v)
{
    m_value = v;
    EmitEvent<UpdateValue>(*this);
}

std::string TextParameter::defaultValue() const { return m_default_value; }

void TextParameter::setDefaultValue(const std::string &v)
{
    m_default_value = v;
    EmitEvent<UpdateSpecification>(*this);
}

ParameterWidget *TextParameter::newWidget(QWidget * parent)
{
    return new ParameterTextWidget(this, parent);
}

void TextParameter::load(const QSettings *setting)
{
    setValue(setting->value(QString::fromStdString(name())).toString().toStdString());
}

void TextParameter::save(QSettings *setting) const
{
        setting->setValue(QString::fromStdString(name()), QString::fromStdString(value()));
}