#include "widget.h"
#include "parameter.h"

#include <QtWidgets/QWidget>


ParameterSelectWidget::ParameterSelectWidget(Parameter *param, QWidget *parent)
    : ParameterWidget(param, parent)
{
    m_selectParam = static_cast<SelectParameter *>(param);

    m_comboBox = new QComboBox();
    m_layout->addWidget(m_comboBox);

    updateWidget(*param);

    QObject::connect(
        m_comboBox, QOverload<const QString &>::of(&QComboBox::activated),
        [&, p = m_selectParam](const QString &text) { p->setValue(text.toStdString()); });
}

void ParameterSelectWidget::updateWidget(const Parameter &p)
{
    const SelectParameter *sp = static_cast<const SelectParameter *>(&p);

    m_comboBox->clear();
    for (auto &v : sp->choices())
        m_comboBox->addItem(QString::fromStdString(v));
    if (sp->tooltips().size() == m_comboBox->count())
        for (int i = 0; i < m_comboBox->count(); ++i)
            m_comboBox->setItemData(i, QString::fromStdString(sp->tooltips()[i]),
                                    Qt::ToolTipRole);

    if (!sp->value().empty())
        m_comboBox->setCurrentText(QString::fromStdString(sp->value()));
    else
        m_comboBox->setCurrentText(QString::fromStdString(sp->defaultValue()));
}