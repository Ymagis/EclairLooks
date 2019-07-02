#pragma once

#include <QtWidgets/QWidget>

#include <utils/generic.h>


class Image;
class ImageWidget;
class PipelineWidget;
class WaveformWidget;
class NeutralWidget;
class CubeWidget;
class OperatorListWidget;
class BrowserWidget;
class QTabBar;
class QStackedWidget;
class VectorScopeWidget;

class DevWidget : public QWidget
{
  public:
    DevWidget(QWidget *parent = nullptr);

  public:
    void showEvent(QShowEvent *event) override;

  public:
    QStackedWidget *operatorWidget();
    PipelineWidget *pipelineWidget();

  private:
    void setupBrowser();
    void setupPipelineView();
    void setupOperatorsView();
    void setupScopeView();
    QWidget * setupUi();

    void updateScope(const Image &img);

  private:
    ImageWidget *m_imageWidget;
    PipelineWidget *m_pipelineWidget;
    QStackedWidget *m_operatorWidget;
    OperatorListWidget *m_operatorsWidget;

    QStackedWidget *m_scopeStack;
    QTabBar *m_scopeTab;
    WaveformWidget *m_waveformWidget;
    VectorScopeWidget *m_vectorscopeWidget;
    NeutralWidget *m_neutralsWidget;
    CubeWidget *m_cubeWidget;

    BrowserWidget *m_lookBrowser;
    BrowserWidget *m_imageBrowser;

    UPtr<Image> m_imageRamp;
    UPtr<Image> m_imageLattice;
    UPtr<Image> m_imageCompute;
};