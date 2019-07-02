#include "widget.h"
#include "parameter.h"


ParameterCheckBoxWidget::ParameterCheckBoxWidget(Parameter *param, QWidget *parent)
    : ParameterWidget(param, parent)
{
    m_checkBoxParam = static_cast<CheckBoxParameter *>(param);

    m_checkBox = new QCheckBox();
    m_layout->addWidget(m_checkBox);

    updateWidget(*param);

    QObject::connect(m_checkBox, &QCheckBox::clicked,
                        [&, p = m_checkBoxParam, cb = m_checkBox]() {
                            p->setValue(cb->isChecked() ? true : false);
                        });
}

void ParameterCheckBoxWidget::updateWidget(const Parameter &p)
{
    const CheckBoxParameter *cbp = static_cast<const CheckBoxParameter *>(&p);
    m_checkBox->setCheckState(cbp->value() ? Qt::Checked : Qt::Unchecked);
}
