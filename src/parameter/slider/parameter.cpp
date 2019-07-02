#include "parameter.h"
#include "widget.h"

#include <QtCore/QSettings>


SliderParameter::SliderParameter(const std::string &name, float value, float min,
                                 float max, float step, Scale scale, Legend legend)
    : Parameter(name), m_value(value), m_default_value(value), m_min(min), m_max(max),
      m_step(step), m_scale(scale), m_legend(legend)
{
}

float SliderParameter::value() const { return m_value; }

void SliderParameter::setValue(const float &v)
{
    m_value = v;
    EmitEvent<UpdateValue>(*this);
}

float SliderParameter::defaultValue() const { return m_default_value; }

void SliderParameter::setDefaultValue(const float &v)
{
    m_default_value = v;
    EmitEvent<UpdateSpecification>(*this);
}

float SliderParameter::min() const { return m_min; }

void SliderParameter::setMin(const float &v)
{
    m_min = v;
    EmitEvent<UpdateSpecification>(*this);
}

float SliderParameter::max() const { return m_max; }

void SliderParameter::setMax(const float &v)
{
    m_max = v;
    EmitEvent<UpdateSpecification>(*this);
}

float SliderParameter::step() const { return m_step; }

void SliderParameter::setStep(const float &v)
{
    m_step = v;
    EmitEvent<UpdateSpecification>(*this);
}

Scale SliderParameter::scale() const { return m_scale; }

void SliderParameter::setScale(const Scale &v)
{
    m_scale = v;
    EmitEvent<UpdateSpecification>(*this);
}

SliderParameter::Legend SliderParameter::legend() const { return m_legend; }

void SliderParameter::setLegend(const Legend &v)
{
    m_legend = v;
}

ParameterWidget *SliderParameter::newWidget(QWidget * parent)
{
    return new ParameterSliderWidget(this, parent);
}

void SliderParameter::load(const QSettings *setting)
{
    setValue(setting->value(QString::fromStdString(name())).toFloat());
}

void SliderParameter::save(QSettings *setting) const
{
    setting->setValue(QString::fromStdString(name()), value());
}