#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QSlider>

#include <core/types.h>


class Slider;
class QLineEdit;
class QDoubleValidator;

// ----------------------------------------------------------------------------

class SliderField : public QWidget
{
  Q_OBJECT

  public:
    SliderField(Qt::Orientation = Qt::Horizontal, Scale s = Scale::Linear, QWidget *parent = nullptr);

  // Slider API is repeated here for convenience
  public:
    float value() const;
    float value(int v) const;
    void setValue(float v);

    void setMinimum(float v);
    float minimum() const;
    void setMaximum(float v);
    float maximum() const;
    void setSingleStep(float v);
    float singleStep() const;
    void setScale(Scale s);
    Scale scale() const;

    void setShowGradation(bool v);
    bool showGradation() const;

  signals:
    void valueChanged(float newValue);

  private:
    Slider *m_slider;
    QLineEdit *m_ledit;
    QDoubleValidator *m_validator;
};

// ----------------------------------------------------------------------------

class Slider : public QSlider
{
  public:
    Slider(Qt::Orientation = Qt::Horizontal, Scale s = Scale::Linear, QWidget *parent = nullptr);

  public:
    float value() const;
    float value(int v) const;
    void setValue(float v);

    void setMinimum(float v);
    float minimum() const;
    void setMaximum(float v);
    float maximum() const;
    void setSingleStep(float v);
    float singleStep() const;
    void setScale(Scale s);
    Scale scale() const;

    void setShowGradation(bool v);
    bool showGradation() const;

  protected:
    void paintEvent(QPaintEvent *event);

  private:
    void _update();

  private:
    float m_min = 0;
    float m_max = 100;
    float m_step = 1;
    Scale m_scale = Scale::Linear;
    bool m_showGradation = true;
};
