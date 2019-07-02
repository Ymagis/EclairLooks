#include "tabview.h"

#include <QtWidgets/QtWidgets>

#include <context.h>
#include <gui/mainwindow.h>
#include "listview.h"
#include "widget.h"


LookViewTabWidget::LookViewTabWidget(QWidget *parent)
:   QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideNone);

    QObject::connect(this, &QTabWidget::currentChanged, this, &LookViewTabWidget::tabChanged);
    QObject::connect(this, &QTabWidget::tabCloseRequested, this, &LookViewTabWidget::tabClosed);
}

LookViewWidget *LookViewTabWidget::currentView()
{
    if (LookViewWidget *w = static_cast<LookViewWidget*>(currentWidget()))
        return w;

    return nullptr;
}

void LookViewTabWidget::setLookWidget(LookWidget *lw)
{
    m_lookWidget = lw;
}

void LookViewTabWidget::showFolder(const QString &path)
{
    QFileInfo fileInfo(path);
    QString dirPath;
    if (fileInfo.isDir())
        dirPath = fileInfo.absoluteFilePath();
    else
        dirPath = fileInfo.dir().absolutePath();

    QString basePath = QString::fromStdString(
        Context::getInstance().settings()
        .Get<FilePathParameter>("Look Base Folder")->value()
    );

    QDir rootDir(basePath);
    QString relPath = rootDir.relativeFilePath(dirPath);

    if (auto [exists, index] = tabExists(relPath); !exists) {
        LookViewWidget *lookViewWidget = new LookViewWidget();
        lookViewWidget->setLookWidget(m_lookWidget);
        lookViewWidget->appendFolder(dirPath);

        if (lookViewWidget->countLook() >= 1) {
            addTab(lookViewWidget, relPath);
            setCurrentIndex(count() - 1);

            QObject::connect(lookViewWidget, &QListWidget::itemSelectionChanged, this, &LookViewTabWidget::selectionChanged);
        }
        else {
            delete lookViewWidget;
        }
    }
    else {
        setCurrentIndex(index);
    }
}

void LookViewTabWidget::updateViews()
{
    // Maybe update current tab first (top priority)...
    for (uint16_t i = 0; i < count(); ++i) {
        LookViewWidget *w = static_cast<LookViewWidget*>(widget(i));
        w->updateView();
    }
}

void LookViewTabWidget::selectionChanged()
{
    LookViewWidget *widget = static_cast<LookViewWidget*>(currentWidget());
    if (widget) {
        QString path = widget->currentLook();
        if (!path.isEmpty()) {
            EmitEvent<Select>(path);
            return;
        }
    }

    EmitEvent<Reset>();
}

void LookViewTabWidget::tabChanged(int index)
{
    selectionChanged();
}

void LookViewTabWidget::tabClosed(int index)
{
    removeTab(index);
}

TupleT<bool, uint16_t> LookViewTabWidget::tabExists(const QString &name)
{
    for (uint16_t i = 0; i < count(); ++i)
        if (tabText(i) == name)
            return { true, i };

    return { false, 0 };
}
