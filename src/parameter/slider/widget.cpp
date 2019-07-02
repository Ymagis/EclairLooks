#include "widget.h"
#include "parameter.h"


ParameterSliderWidget::ParameterSliderWidget(Parameter *param, QWidget *parent)
    : ParameterWidget(param, parent)
{
    m_sliderParam = static_cast<SliderParameter *>(param);

    m_slider = new SliderField(Qt::Horizontal, Scale::Linear);
    m_layout->addWidget(m_slider);

    updateWidget(*param);

    QObject::connect(m_slider, &SliderField::valueChanged,
                        [&, p = m_sliderParam](float value) {
                            p->setValue(value);
                        });
}

SliderField* ParameterSliderWidget::sliderField()
{
    return m_slider;
}

void ParameterSliderWidget::updateWidget(const Parameter &p)
{
    const SliderParameter *sp = static_cast<const SliderParameter *>(&p);

    m_slider->setMinimum(sp->min());
    m_slider->setMaximum(sp->max());
    m_slider->setSingleStep(sp->step());
    m_slider->setScale(sp->scale());
    m_slider->setValue(sp->value());
    m_slider->setShowGradation(sp->legend() == SliderParameter::Legend::ShowTicks);
}