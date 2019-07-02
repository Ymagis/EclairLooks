#pragma once

#include <string>

#include <core/types.h>
#include "../parameter.h"


class SliderParameter : public Parameter
{
  public:
    enum class Legend { ShowTicks, None };

  public:
    SliderParameter() = default;
    SliderParameter(const std::string &name, float value, float min, float max,
                    float step, Scale scale = Scale::Linear,
                    Legend legend = Legend::ShowTicks);

  public:
    float value() const;
    void setValue(const float &v);

    float defaultValue() const;
    void setDefaultValue(const float &v);

    float min() const;
    void setMin(const float &v);

    float max() const;
    void setMax(const float &v);

    float step() const;
    void setStep(const float &v);

    Scale scale() const;
    void setScale(const Scale &v);

    Legend legend() const;
    void setLegend(const Legend &v);

  public:
    ParameterWidget *newWidget(QWidget * parent = nullptr) override;

    void load(const QSettings *setting) override;
    void save(QSettings *setting) const override;

  private:
    float m_value;
    float m_default_value;
    float m_min;
    float m_max;
    float m_step;
    Scale m_scale = Scale::Linear;
    Legend m_legend = Legend::ShowTicks;
};