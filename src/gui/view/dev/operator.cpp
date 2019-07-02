#include "operator.h"

#include <QtWidgets/QtWidgets>

#include <operator/imageoperator.h>
#include <parameter/parameterwidget.h>


OperatorWidget::OperatorWidget(ImageOperator *op, QWidget *parent)
:   QTabWidget(parent), m_operator(op)
{
    setupUi();
}

void OperatorWidget::setupUi()
{
    for (auto &[cat, plist] : m_operator->Categories()) {
        QWidget *tab = new QWidget();
        QFormLayout *formLayout = new QFormLayout(tab);
        formLayout->setVerticalSpacing(2);
        formLayout->setHorizontalSpacing(6);
        formLayout->setContentsMargins(6, 6, 6, 6);
        formLayout->setLabelAlignment(Qt::AlignLeft);
        formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        for (auto & name : plist)
            for (auto & p : m_operator->Parameters())
                if (p->name() == name) {
                    ParameterWidget *paramWidget = p->createWidget();
                    paramWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

                    if (p->displayName() != "")
                        formLayout->addRow(new QLabel(QString::fromStdString(p->displayName())), paramWidget);
                    else
                        formLayout->addRow(paramWidget);
                }

        addTab(tab, QString::fromStdString(cat));

        if (cat == m_operator->DefaultCategory())
            tabBar()->moveTab(count() - 1, 0);
    }
}
