#include "setting.h"

#include <QtWidgets/QtWidgets>

#include <parameter/parameterseriallist.h>
#include <parameter/parameterwidget.h>


SettingWidget::SettingWidget(ParameterSerialList *settings, const QString &header, QWidget *parent)
    : QWidget(parent), m_settings(settings), m_headerName(header)
{
    QFormLayout *fLayout = new QFormLayout(this);
    fLayout->setContentsMargins(4, 4, 4, 4);
    fLayout->setLabelAlignment(Qt::AlignLeft);
    fLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    if (!m_headerName.isEmpty())
        fLayout->addRow("<b>" + m_headerName + "</b>", new QWidget());

    for (auto & p : m_settings->Parameters()) {
        QLabel * label = new QLabel(QString::fromStdString(p->displayName()));
        ParameterWidget *paramWidget = p->createWidget(this);
        paramWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        fLayout->addRow(label, paramWidget);
    }
}
