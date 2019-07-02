#include "uiloader.h"

#include <gui/common/browser.h>
#include <gui/common/imageviewer.h>
#include <gui/view/dev/pipeline.h>
#include <gui/view/dev/operatorlist.h>
#include <gui/view/look/tabview.h>
#include <gui/view/look/detail.h>
#include <gui/view/look/selection.h>
#include <gui/scope/waveform.h>


UiLoader::UiLoader(QObject *parent) : QUiLoader(parent) {}

QWidget *UiLoader::createWidget(const QString &className, QWidget *parent,
                                const QString &name)
{
    if (className == "ImageWidget") {
        ImageWidget *w = new ImageWidget(parent);
        w->setObjectName(name);
        return w;
    } else if (className == "PipelineWidget") {
        PipelineWidget *w = new PipelineWidget(parent);
        w->setObjectName(name);
        return w;
    }  else if (className == "OperatorListWidget") {
        OperatorListWidget *w = new OperatorListWidget(parent);
        w->setObjectName(name);
        return w;
    } else if (className == "BrowserWidget") {
        BrowserWidget *w = new BrowserWidget(parent);
        w->setObjectName(name);
        return w;
    } else if (className == "LookViewTabWidget") {
        LookViewTabWidget *w = new LookViewTabWidget(parent);
        w->setObjectName(name);
        return w;
    } else if (className == "LookDetailWidget") {
        LookDetailWidget *w = new LookDetailWidget(parent);
        w->setObjectName(name);
        return w;
    } else if (className == "LookSelectionWidget") {
        LookSelectionWidget *w = new LookSelectionWidget(parent);
        w->setObjectName(name);
        return w;
    } else if (className == "QTabBar") {
        QTabBar *w = new QTabBar(parent);
        w->setObjectName(name);
        return w;
    }

    return QUiLoader::createWidget(className, parent, name);
}