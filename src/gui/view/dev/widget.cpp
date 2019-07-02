#include "widget.h"

#include <QtWidgets/QtWidgets>
#include <QFile>

#include <context.h>
#include <core/imagepipeline.h>
#include <operator/imageoperatorlist.h>
#include <gui/common/browser.h>
#include <gui/common/imageviewer.h>
#include <gui/mainwindow.h>
#include <gui/uiloader.h>
#include <gui/scope/waveform.h>
#include <gui/scope/neutral.h>
#include <gui/scope/cube.h>
#include <gui/scope/vectorscope.h>
#include "pipeline.h"
#include "operator.h"
#include "operatorlist.h"


DevWidget::DevWidget(QWidget *parent)
    : QWidget(parent)
{
    m_imageRamp = std::make_unique<Image>(Image::Ramp1D(4096));
    m_imageLattice = std::make_unique<Image>(Image::Lattice(17));
    m_imageCompute = std::make_unique<Image>();

    //
    // Setup
    //

    QWidget * w = setupUi();
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(w);
    setLayout(layout);

    m_imageWidget = findChild<ImageWidget*>("imageWidget");
    m_pipelineWidget = findChild<PipelineWidget*>("pipelineWidget");
    m_operatorWidget = findChild<QStackedWidget*>("operatorDetailWidget");
    m_operatorsWidget = findChild<OperatorListWidget*>("operatorListWidget");
    m_scopeStack = findChild<QStackedWidget*>("scopeStack");
    m_scopeTab = findChild<QTabBar*>("scopeBar");
    m_lookBrowser = findChild<BrowserWidget*>("lookBrowserWidget");
    m_imageBrowser = findChild<BrowserWidget*>("imageBrowserWidget");

    //There might be a memleak here !!!! 

    m_waveformWidget = new WaveformWidget();
    m_neutralsWidget = new NeutralWidget();
    m_cubeWidget = new CubeWidget();
    m_vectorscopeWidget = new VectorScopeWidget();

    // NOTE : see https://stackoverflow.com/a/43835396/4814046
    QSplitter *vSplitter = findChild<QSplitter*>("vSplitter");
    vSplitter->setSizes(QList<int>({65000, 35000}));
    QSplitter *hSplitterTop = findChild<QSplitter*>("hSplitterTop");
    hSplitterTop->setSizes(QList<int>({15000, 15000, 70000}));
    QSplitter *hSplitterBottom = findChild<QSplitter*>("hSplitterBottom");
    hSplitterBottom->setSizes(QList<int>({15000, 55000, 30000}));

    setupPipelineView();
    setupScopeView();
    setupOperatorsView();
    setupBrowser();

    //
    // Connections
    //

    using std::placeholders::_1;
    using IP = ImagePipeline;
    using IW = ImageWidget;
    using BW = BrowserWidget;
    using P = Parameter;

    ImagePipeline& pipeline = Context::getInstance().pipeline();
    ParameterSerialList& settings = Context::getInstance().settings();

    pipeline.Subscribe<IP::NewInput>(std::bind(&ImageWidget::resetImage, m_imageWidget, _1));
    pipeline.Subscribe<IP::Update>(std::bind(&ImageWidget::updateImage, m_imageWidget, SideBySide::A, _1));
    pipeline.Subscribe<IP::Update>(std::bind(&DevWidget::updateScope, this, _1));

    m_imageWidget->Subscribe<IW::DropImage>(std::bind(&ImagePipeline::SetInput, &pipeline, _1));

    auto lookRootPath = settings.Get<FilePathParameter>("Look Base Folder");
    lookRootPath->Subscribe<P::UpdateValue>([this](auto &p){
        m_lookBrowser->setRootPath(
            QString::fromStdString(
                static_cast<const FilePathParameter&>(p).value()
            )
        );
    });
    auto imageRootPath = settings.Get<FilePathParameter>("Image Base Folder");
    imageRootPath->Subscribe<P::UpdateValue>([this](auto &p){
        m_imageBrowser->setRootPath(
            QString::fromStdString(
                static_cast<const FilePathParameter&>(p).value()
            )
        );
    });

    m_lookBrowser->Subscribe<BW::Select>([this](const QString &path) {
        m_pipelineWidget->addFromFile(path.toStdString());
    });
    m_imageBrowser->Subscribe<BW::Select>([&pipeline](const QString &path) {
        pipeline.SetInput(Image::FromFile(path.toStdString()));
    });
}

void DevWidget::showEvent(QShowEvent *event)
{
    // When we first switch on this widget, OpenGL context is initialized
    // All update event prior to this did nothing so we need to redraw scopes
    updateScope(Context::getInstance().pipeline().GetOutput());
}

QStackedWidget* DevWidget::operatorWidget()
{
    return m_operatorWidget;
}

PipelineWidget *DevWidget::pipelineWidget()
{
    return m_pipelineWidget;
}

QWidget* DevWidget::setupUi()
{
    UiLoader loader;
    QFile file(":/ui/devwidget.ui");
    file.open(QFile::ReadOnly);
    return loader.load(&file, this);
}

void DevWidget::setupBrowser()
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

void DevWidget::setupPipelineView()
{
    m_pipelineWidget->setPipeline(&Context::getInstance().pipeline());
    m_pipelineWidget->setDevWidget(this);
}

void DevWidget::setupScopeView()
{
    m_scopeStack->addWidget(m_waveformWidget);
    m_scopeStack->addWidget(m_neutralsWidget);
    m_scopeStack->addWidget(m_cubeWidget);
    m_scopeStack->addWidget(m_vectorscopeWidget);
    m_scopeStack->setCurrentWidget(m_waveformWidget);

    // NOTE : ideally we should make no assumptions of what scope mode are
    // available and discover them from the ScopeWidget class directly.
    m_scopeTab->setExpanding(false);
    m_scopeTab->addTab("W");
    m_scopeTab->addTab("P");
    m_scopeTab->addTab("N");
    m_scopeTab->addTab("C");
    m_scopeTab->addTab("V");

    QObject::connect(
        m_scopeTab, &QTabBar::tabBarClicked,
        [&, this](int index) {
            QString tabText = m_scopeTab->tabText(index);

            if (tabText == "W") {
                m_scopeStack->setCurrentWidget(m_waveformWidget);
                m_waveformWidget->setScopeType("Waveform");
            }
            else if (tabText == "P") {
                m_scopeStack->setCurrentWidget(m_waveformWidget);
                m_waveformWidget->setScopeType("Parade");
            }
            else if (tabText == "N") {
                m_scopeStack->setCurrentWidget(m_neutralsWidget);
            }
            else if (tabText == "C") {
                m_scopeStack->setCurrentWidget(m_cubeWidget);
            }
            else if (tabText == "V") {
                m_scopeStack->setCurrentWidget(m_vectorscopeWidget);
            }

            // Need to manually update the scope because it's not updated when not visible.
            updateScope(Context::getInstance().pipeline().GetOutput());
        }
    );
}

void DevWidget::setupOperatorsView()
{
    m_operatorsWidget->setDevWidget(this);
}

void DevWidget::updateScope(const Image &img)
{
    ImagePipeline& pipeline = Context::getInstance().pipeline();

    if (m_scopeStack->currentWidget() == m_neutralsWidget) {
        qInfo() << "Compute Ramp (curve scope)";
        *m_imageCompute = *m_imageRamp;
        pipeline.ComputeImage(*m_imageCompute);
        m_neutralsWidget->drawCurve(0, *m_imageCompute);
    }
    else if (m_scopeStack->currentWidget() == m_cubeWidget) {
        qInfo() << "Compute Lattice (cube scope)";
        *m_imageCompute = *m_imageLattice;
        pipeline.ComputeImage(*m_imageCompute);
        m_cubeWidget->drawCube(*m_imageCompute);
    }
    else if (m_scopeStack->currentWidget() == m_waveformWidget) {
        m_waveformWidget->updateTexture(m_imageWidget->texture());
    }
    else if (m_scopeStack->currentWidget() == m_vectorscopeWidget) {
        m_vectorscopeWidget->updateTexture(m_imageWidget->texture());
    }
}