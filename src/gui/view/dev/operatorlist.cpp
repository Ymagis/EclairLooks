#include "operatorlist.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtGui/QDragEnterEvent>
#include <QtWidgets/QtWidgets>

#include <context.h>
#include "widget.h"
#include "pipeline.h"


OperatorListWidget::OperatorListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);

    QObject::connect(this, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
        if (!m_devWidget)
            return;

        m_devWidget->pipelineWidget()->addFromName(item->text().toStdString());
    });
}

void OperatorListWidget::startDrag(Qt::DropActions supportedActions)
{
    QListWidgetItem *child = currentItem();
    if (!child)
        return;

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(child->text());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

void OperatorListWidget::setDevWidget(DevWidget *w)
{
    m_devWidget = w;

    clear();
    for (auto &name : Context::getInstance().operators().Operators())
        addItem(new QListWidgetItem(QString::fromStdString(name)));
}
