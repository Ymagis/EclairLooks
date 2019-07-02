#include "parameter.h"
#include "widget.h"

#include <utils/pystring.h>


MatrixParameter::MatrixParameter(const std::string &name) : Parameter(name)
{
    m_value = m_default_value;
}

Matrix4x4 MatrixParameter::value() const { return m_value; }

void MatrixParameter::setValue(const Matrix4x4 &v)
{
    m_value = v;
    EmitEvent<UpdateValue>(*this);
}

Matrix4x4 MatrixParameter::defaultValue() const { return m_default_value; }

void MatrixParameter::setDefaultValue(const Matrix4x4 &v)
{
    m_default_value = v;
    EmitEvent<UpdateValue>(*this);
}

ParameterWidget *MatrixParameter::newWidget(QWidget * parent)
{
    return new ParameterMatrixWidget(this, parent);
}

void MatrixParameter::load(const QSettings *setting)
{
    std::string valueStr = setting->value(QString::fromStdString(name())).toString().toStdString();

    Matrix4x4 m;
    std::vector<std::string> numbers;
    pystring::split(valueStr, numbers, " ");

    if (numbers.size() != 16) {
        qWarning() << "Invalid Matrix settings, expecting 16 numbers";
        return;
    }

    for (int i = 0; i < numbers.size(); ++i)
        m[i] = std::stof(numbers[i]);

    setValue(m);
}

void MatrixParameter::save(QSettings *setting) const
{
    std::string valueStr;
    for (auto v : value())
        valueStr += std::to_string(v) + " ";

    setting->setValue(QString::fromStdString(name()), QString::fromStdString(valueStr));
}
