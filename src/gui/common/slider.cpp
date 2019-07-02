#include "slider.h"

#include <cmath>

#include <QtWidgets/QtWidgets>


// ----------------------------------------------------------------------------

SliderField::SliderField(Qt::Orientation orientation, Scale s, QWidget *parent)
: QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_slider = new Slider(orientation);
    m_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    layout->addWidget(m_slider);

    m_ledit = new QLineEdit();
    m_ledit->setAlignment(Qt::AlignCenter);
    m_ledit->setFixedWidth(m_ledit->fontMetrics().boundingRect("0000001").width());
    layout->addWidget(m_ledit);

    m_validator = new QDoubleValidator(m_ledit);
    m_validator->setDecimals(5);
    m_validator->setNotation(QDoubleValidator::StandardNotation);
    m_ledit->setValidator(m_validator);

    m_ledit->setText(QString::number(value(), 'G', 5));

    QObject::connect(m_slider, QOverload<int>::of(&QSlider::valueChanged),
                     [&, le = m_ledit](int value) {
                         le->setText(QString::number(value, 'G', 5));
                         emit valueChanged(SliderField::value());
                     });

    QObject::connect(m_ledit, QOverload<>::of(&QLineEdit::returnPressed),
                     [&, le = m_ledit]() {
                         setValue(le->text().toFloat());
                         emit valueChanged(SliderField::value());
                     });
}

float SliderField::value() const { return m_slider->value(); }

float SliderField::value(int v) const { return m_slider->value(v); }

void SliderField::setValue(float v) { m_slider->setValue(v); }

void SliderField::setMinimum(float v)
{
    m_slider->setMinimum(v);
    m_validator->setRange(m_slider->minimum(), m_slider->maximum());
}

float SliderField::minimum() const { return m_slider->minimum(); }

void SliderField::setMaximum(float v)
{
    m_slider->setMaximum(v);
    m_validator->setRange(m_slider->minimum(), m_slider->maximum());
}

float SliderField::maximum() const { return m_slider->maximum(); }

void SliderField::setSingleStep(float v) { m_slider->setSingleStep(v); }

float SliderField::singleStep() const { return m_slider->singleStep(); }

void SliderField::setScale(Scale s) { m_slider->setScale(s); }

Scale SliderField::scale() const { return m_slider->scale(); }

void SliderField::setShowGradation(bool v) { m_slider->setShowGradation(v); }

bool SliderField::showGradation() const { return m_slider->showGradation(); }

// ----------------------------------------------------------------------------

Slider::Slider(Qt::Orientation orientation, Scale s, QWidget *parent)
: QSlider(orientation, parent), m_scale(s)
{
    setMinimumHeight(25);
    setTickInterval(25);
    QSlider::setMinimum(0);
    QSlider::setMaximum(100);
    QSlider::setSingleStep(1);
}

float Slider::value() const
{
    int val = QSlider::value();
    return value(val);
}

float Slider::value(int val) const
{
    switch (m_scale) {
        case Scale::Linear: {
            return std::min(m_min + val * m_step, m_max);
        }
        case Scale::Log: {
            float minv = std::log(m_min);
            float maxv = std::log(m_max);
            float factor = (maxv - minv) / QSlider::maximum();
            return std::exp(minv + factor * (val - QSlider::minimum()));
        }
    }

    return 0.f;
}

void Slider::setValue(float v)
{
    switch (m_scale) {
        case Scale::Linear: {
            int stepcount = (QSlider::maximum() - QSlider::minimum()) / QSlider::singleStep();
            float val = (v - m_min) / (m_max - m_min);
            QSlider::setValue(val * stepcount);
        }
        break;
        case Scale::Log: {
            float minv = std::log(m_min);
            float maxv = std::log(m_max);
            float factor = (maxv - minv) / QSlider::maximum();
            int val = (std::log(v) - minv) / factor + QSlider::minimum();
            QSlider::setValue(val);
        }
        break;
    }
}

void Slider::setMinimum(float v)
{
    m_min = v;
    _update();
}

float Slider::minimum() const
{
    return m_min;
}

void Slider::setMaximum(float v)
{
    m_max = v;
    _update();
}

float Slider::maximum() const
{
    return m_max;
}

void Slider::setSingleStep(float v)
{
    m_step = v;
    _update();
}

float Slider::singleStep() const
{
    return m_step;
}

void Slider::setScale(Scale s)
{
    m_scale = s;
    _update();
}

Scale Slider::scale() const
{
    return m_scale;
}

void Slider::setShowGradation(bool v)
{
    m_showGradation = v;
    update();
}

bool Slider::showGradation() const
{
    return m_showGradation;
}

void Slider::_update()
{
    if (m_max <= m_min)
        return;
    if (m_step <= 0)
        return;

    switch(m_scale) {
        case Scale::Linear: {
            int requiredStep = (m_max - m_min) / m_step;
            QSlider::setMinimum(0);
            QSlider::setMaximum(requiredStep);
            QSlider::setSingleStep(1);
        }
        break;
        case Scale::Log: {
            QSlider::setMinimum(0);
            QSlider::setMaximum(1000);
            QSlider::setSingleStep(1);
        }
        break;
    }
}

void Slider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);

    if (!m_showGradation)
        return;

    QPainter painter(this);
    QPen pen;
    pen.setWidth(1);

    uint8_t xmargin = 5;
    uint8_t tick_width = 1;
    uint8_t tick_height = 8;
    uint8_t subtick_count = 5;
    uint8_t subtick_height = 3;

    // Draw legend
    QFont font = painter.font();
    font.setPointSize(font.pointSize() - 3);
    painter.setFont(font);
    QFontMetrics metric(font);

    int min = QSlider::minimum();
    int max = QSlider::maximum();
    int step = QSlider::tickInterval();
    const int maxtick = 4;
    if ((max - min) / step > maxtick)
        step = (max - min) / maxtick;

    for (int v = min; v <= max; v += step) {
        int x = xmargin + width() * (1.f * v / max);
        x = std::min(x, width() - xmargin);

        // Draw ticks above
        pen.setColor(QColor(Qt::darkGray));
        painter.setPen(pen);
        painter.drawRect(x, height() / 2. - tick_height, tick_width, tick_height);

        // Draw intermediate ticks
        pen.setColor(QColor(Qt::black));
        for (int i = 1; i < subtick_count; ++i) {
            int xi = xmargin + width() * ((v + step * (1.f * i / subtick_count)) / max);
            xi = std::min(xi, width() - xmargin);
            painter.drawRect(xi, height() / 2 - subtick_height, tick_width, subtick_height);
        }

        // Draw text below
        QString txt = QString::number(value(v), 'G', 5);
        QRect rect = metric.boundingRect(txt);
        x = x - rect.width() / 2;
        x = std::min(x, width() - rect.width());
        x = std::max(0, x);

        pen.setColor(QColor(Qt::gray));
        painter.setPen(pen);
        painter.drawText(x, height(), txt);
    }
}