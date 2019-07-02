#include "parameter.h"
#include "parameterwidget.h"

using std::placeholders::_1;


Parameter::Parameter(const std::string &name) : m_name(name), m_display_name(name) {}

Parameter::operator bool() const { return m_name != ""; }

std::string Parameter::name() const { return m_name; }

std::string Parameter::displayName() const { return m_display_name; }

void Parameter::setDisplayName(const std::string &v) { m_display_name = v; }

ParameterWidget *Parameter::createWidget(QWidget *parent)
{
    ParameterWidget* w = newWidget(parent);

    auto c1 = Subscribe<Parameter::UpdateValue>(std::bind(&ParameterWidget::updateWidget, w, _1));
    auto c2 = Subscribe<Parameter::UpdateSpecification>(std::bind(&ParameterWidget::updateWidget, w, _1));

    QObject::connect(w, &QWidget::destroyed, [&, this, c1, c2]() {
        Unsubscribe<Parameter::UpdateValue>(c1);
        Unsubscribe<Parameter::UpdateSpecification>(c2);
    });

    return w;
}