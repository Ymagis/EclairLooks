#include "widget.h"

#include <QtWidgets/QtWidgets>
#include <QFile>

#include <context.h>
#include <core/types.h>
#include <core/image.h>
#include <operator/ocio/filetransform.h>
#include <gui/mainwindow.h>
#include <gui/uiloader.h>
#include <gui/common/browser.h>
#include <gui/common/setting.h>
#include "tabview.h"
#include "listview.h"
#include "detail.h"
#include "selection.h"


using std::placeholders::_1;
using BW = BrowserWidget;
using LV = LookViewTabWidget;
using LD = LookDetailWidget;
using P = Parameter;

// TODO : fix memory leak on m_settings, should not be that problematic because
// it only leaks on application exit, hence when memory is reclaimed anyway.
// Cannot delete it right in the destructor because SettingWidget
// (and children ParameterWidget) use it on destruction to unconnect callbacks.
// These widget are destructed in the base class QObject / QWidget, after
// LookWidget destructor, signal QWidget::destroyed is not a solution either
// because it's called before destruction.
LookWidget::LookWidget(QWidget *parent)
    : QWidget(parent), m_settings(new ParameterSerialList()), m_proxySize(125, 70)
{
    m_pipeline = std::make_unique<ImagePipeline>();
    m_pipeline->SetName("look");
    m_imageRamp = std::make_unique<Image>(Image::Ramp1D(4096));
    m_imageLattice = std::make_unique<Image>(Image::Lattice(17));

    //
    // Setup
    //

    QWidget * w = setupUi();
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(w);

    m_lookBrowser = findChild<BrowserWidget*>("lookBrowserWidget");
    m_imageBrowser = findChild<BrowserWidget*>("imageBrowserWidget");
    m_viewTabWidget = findChild<LookViewTabWidget*>("lookViewWidget");
    m_detailWidget = findChild<LookDetailWidget*>("lookDetailWidget");
    m_selectWidget = findChild<LookSelectionWidget*>("lookSelectionWidget");
    m_settingWidget = findChild<QWidget*>("lookSettingWidget");

    // NOTE : see https://stackoverflow.com/a/43835396/4814046
    m_vSplitter = findChild<QSplitter*>("hSplitter");
    m_vSplitter->setSizes(QList<int>({15000, 85000}));
    m_hSplitter = findChild<QSplitter*>("vSplitter");
    m_hSplitter->setSizes(QList<int>({60000, 40000}));
    m_hSplitterView = findChild<QSplitter*>("hSplitterView");
    m_hSplitterView->setSizes(QList<int>({80000, 20000}));

    setupPipeline();
    setupBrowser();
    setupSetting();

    updateToneMap();

    m_viewTabWidget->setLookWidget(this);
    m_detailWidget->setLookWidget(this);
    m_selectWidget->setLookWidget(this);

    //
    // Connections
    //

    ParameterSerialList& settings = Context::getInstance().settings();

    auto lookRootPath = settings.Get<FilePathParameter>("Look Base Folder");
    lookRootPath->Subscribe<P::UpdateValue>([this](auto &p){
        m_lookBrowser->setRootPath(
            QString::fromStdString(
                static_cast<const FilePathParameter&>(p).value()
            )
        );
    });

    auto lookTonemap = settings.Get<FilePathParameter>("Look Tonemap LUT");
    lookTonemap->Subscribe<P::UpdateValue>(std::bind(&LookWidget::updateToneMap, this));

    auto imageRootPath = settings.Get<FilePathParameter>("Image Base Folder");
    imageRootPath->Subscribe<P::UpdateValue>([this](auto &p){
        m_imageBrowser->setRootPath(
            QString::fromStdString(
                static_cast<const FilePathParameter&>(p).value()
            )
        );
    });

    m_lookBrowser->Subscribe<BW::Select>(std::bind(&LV::showFolder, m_viewTabWidget, _1));
    m_imageBrowser->Subscribe<BW::Select>([this](const QString &path) {
        updateImage(Image::FromFile(path.toStdString()));
        resetViews();
        updateViews();
    });

    m_viewTabWidget->Subscribe<LV::Reset>(std::bind(&LD::clearView, m_detailWidget, SideBySide::A));
    m_viewTabWidget->Subscribe<LV::Select>(std::bind(&LD::showDetail, m_detailWidget, _1, SideBySide::A));

    m_selectWidget->Subscribe<LV::Reset>(std::bind(&LD::clearView, m_detailWidget, SideBySide::B));
    m_selectWidget->Subscribe<LV::Select>(std::bind(&LD::showDetail, m_detailWidget, _1, SideBySide::B));
}

bool LookWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Space: {
                toggleFullScreen();
                return true;
            }
            case Qt::Key_Up:
            case Qt::Key_Down: {
                QObject *wantedTarget = m_viewTabWidget->currentView();
                if (keyEvent->modifiers() & Qt::ShiftModifier)
                    wantedTarget = m_selectWidget->viewWidget();

                if (wantedTarget && obj != wantedTarget) {
                    QCoreApplication::sendEvent(wantedTarget, event);
                    return true;
                }
                return false;
            }
            default:
                return false;
        }
    } else {
        return QWidget::eventFilter(obj, event);
    }
}

LookViewTabWidget * LookWidget::lookViewTabWidget()
{
    return m_viewTabWidget;
}

void LookWidget::toggleFullScreen()
{
    if (!m_isFullScreen) {
        m_hSplitterState = m_hSplitter->saveState();
        m_vSplitterState = m_vSplitter->saveState();

        m_vSplitter->setSizes(QList<int>({00000, 100000}));
        m_hSplitter->setSizes(QList<int>({00000, 100000}));

        m_isFullScreen = true;
    }
    else {
        m_hSplitter->restoreState(m_hSplitterState);
        m_vSplitter->restoreState(m_vSplitterState);

        m_isFullScreen = false;

        // User can browse through list while in full screen with shortcuts,
        // we need to re-center the view on current item when fullscreen
        // mode is disabled.
        if (LookViewWidget *view = m_viewTabWidget->currentView())
            view->scrollToItem(view->currentItem(), QAbstractItemView::PositionAtCenter);
    }
}

bool LookWidget::tonemapEnabled() const
{
    return m_settings->Get<CheckBoxParameter>("Tone Mapping")->value();
}

Image & LookWidget::fullImage()
{
    return *m_image;
}

Image & LookWidget::proxyImage()
{
    return *m_imageProxy;
}

TupleT<bool, Image &>LookWidget::lookPreview(const QString &lookPath)
{
    return lookPreview(lookPath, fullImage());
}

TupleT<bool, Image &> LookWidget::lookPreviewProxy(const QString &lookPath)
{
    return lookPreview(lookPath, proxyImage());
}

TupleT<bool, Image &> LookWidget::lookPreviewRamp(const QString &lookPath)
{
    return lookPreview(lookPath, *m_imageRamp);
}

TupleT<bool, Image &> LookWidget::lookPreviewLattice(const QString &lookPath)
{
    return lookPreview(lookPath, *m_imageLattice);
}

TupleT<bool, Image &> LookWidget::lookPreview(const QString &lookPath, Image &img)
{
    ImageOperator &op = m_pipeline->GetOperator(0);
    auto opPath = op.GetParameter<FilePathParameter>("LUT");
    std::string currentPath = opPath->value();
    std::string requestPath = lookPath.toStdString();

    if (currentPath != requestPath)
        opPath->setValue(requestPath);

    m_pipeline->SetInput(img);
    return { true, m_pipeline->GetOutput() };
}

QWidget * LookWidget::setupUi()
{
    UiLoader loader;
    QFile file(":/ui/lookwidget.ui");
    file.open(QFile::ReadOnly);
    return loader.load(&file, this);
}

void LookWidget::setupPipeline()
{
    updateImage(Context::getInstance().pipeline().GetInput());

    m_pipeline->AddOperator<OCIOFileTransform>();
    m_pipeline->AddOperator<OCIOFileTransform>();
}

void LookWidget::setupBrowser()
{
    ParameterSerialList& s = Context::getInstance().settings();

    m_lookBrowser->setRootPath(QString::fromStdString(s.Get<FilePathParameter>("Look Base Folder")->value()));
    m_lookBrowser->setExtFilter(Context::getInstance().supportedLookExtensions());

    QStringList imgExts;
    for (auto ext : Image::SupportedExtensions())
        imgExts << "*." + QString::fromStdString(ext);
    m_imageBrowser->setRootPath(QString::fromStdString(s.Get<FilePathParameter>("Image Base Folder")->value()));
    m_imageBrowser->setExtFilter(imgExts);
}

void LookWidget::setupSetting()
{
    CheckBoxParameter *p = m_settings->Add<CheckBoxParameter>("Tone Mapping", true);
    toggleToneMap(p->value());

    p->Subscribe<Parameter::UpdateValue>([this](const Parameter &p){
        const CheckBoxParameter & cb = static_cast<const CheckBoxParameter&>(p);
        toggleToneMap(cb.value());
    });

    QVBoxLayout *layout = new QVBoxLayout(m_settingWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    SettingWidget *sw = new SettingWidget(m_settings);
    layout->addWidget(sw);
}

void LookWidget::updateImage(const Image &img)
{
    if (!img)
        return;

    m_image = std::make_unique<Image>(img);
    m_imageProxy = std::make_unique<Image>(*m_image);
    *m_imageProxy = m_imageProxy->resize(
        m_proxySize.width(), m_proxySize.height(), true, "box");
}

void LookWidget::resetViews()
{
    m_detailWidget->resetViews();
}

void LookWidget::updateViews()
{
    m_detailWidget->updateView(SideBySide::A);
    m_detailWidget->updateView(SideBySide::B);
    m_viewTabWidget->updateViews();
}

void LookWidget::toggleToneMap(bool v)
{
    ImageOperator &op = m_pipeline->GetOperator(1);
    op.GetParameter<CheckBoxParameter>("Enabled")->setValue(v);

    updateViews();
}

void LookWidget::updateToneMap()
{
    QString path = QString::fromStdString(
        Context::getInstance().settings()
        .Get<FilePathParameter>("Look Tonemap LUT")->value()
    );
    if (path.isEmpty())
        return;

    if (auto op = Context::getInstance().operators().CreateFromPath(path.toStdString())) {
        op = m_pipeline->ReplaceOperator(op, 1);
        op->GetParameter<CheckBoxParameter>("Enabled")->setValue(tonemapEnabled());
    }

    updateViews();
}