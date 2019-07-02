#include "parameterwidget.h"

#include <QtWidgets/QtWidgets>
#include <QtCore/QDebug>


ParameterWidget::ParameterWidget(Parameter *param, QWidget *parent)
:   QWidget(parent), m_param(param), m_layout(nullptr)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
}