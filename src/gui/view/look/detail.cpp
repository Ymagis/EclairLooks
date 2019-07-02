#include "detail.h"

#include <QtWidgets/QtWidgets>

#include <gui/common/imageviewer.h>
#include <gui/scope/neutral.h>
#include <gui/scope/cube.h>
#include "widget.h"


LookDetailWidget::LookDetailWidget(QWidget *parent)
:   QWidget(parent)
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter *hSplitter = new QSplitter(Qt::Horizontal);
    m_imageWidget = new ImageWidget();
    hSplitter->addWidget(m_imageWidget);
    m_neutralsWidget = new NeutralWidget();
    hSplitter->addWidget(m_neutralsWidget);
    m_cubeWidget = new CubeWidget();
    hSplitter->addWidget(m_cubeWidget);

    // NOTE : see https://stackoverflow.com/a/43835396/4814046
    hSplitter->setSizes(QList<int>({33333, 33333, 33333}));

    m_titleLabel = new QLabel();
    m_titleLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    vLayout->addWidget(m_titleLabel);
    vLayout->addWidget(hSplitter);
}

void LookDetailWidget::showEvent(QShowEvent *event)
{
    if (!m_imageWidget->hasImage())
        m_imageWidget->resetImage(m_lookWidget->fullImage());

    QWidget::showEvent(event);
}

void LookDetailWidget::setLookWidget(LookWidget *lw)
{
    m_lookWidget = lw;
    installEventFilter(m_lookWidget);
    m_neutralsWidget->installEventFilter(m_lookWidget);
}

void LookDetailWidget::resetViews()
{
    Image &img  = m_lookWidget->fullImage();
    m_imageWidget->updateImage(SideBySide::A, img);
    m_imageWidget->updateImage(SideBySide::B, img);
}

void LookDetailWidget::clearView(SideBySide c)
{
    m_cmap[c] = "";
    m_titleLabel->setText(m_cmap[SideBySide::A]);

    m_neutralsWidget->clearCurve(UnderlyingT<SideBySide>(c));
    m_imageWidget->updateImage(c, m_lookWidget->fullImage());

    // No comparaison mode for cube at the moment
    if (c == SideBySide::A)
        m_cubeWidget->clearCube();
}

void LookDetailWidget::updateView(SideBySide c)
{
    if (m_cmap[c].isEmpty())
        return;

    showDetail(m_cmap[c], c);
}

void LookDetailWidget::showDetail(const QString &path, SideBySide c)
{
    m_cmap[c] = path;
    m_titleLabel->setText(m_cmap[SideBySide::A]);

    if (auto [valid, img] = m_lookWidget->lookPreview(path); valid)
        m_imageWidget->updateImage(c, img);
    if (auto [valid, img] = m_lookWidget->lookPreviewRamp(path); valid)
        m_neutralsWidget->drawCurve(UnderlyingT<SideBySide>(c), img, path);

    // No comparaison mode for cube at the moment
    if (c == SideBySide::A)
        if (auto [valid, img] = m_lookWidget->lookPreviewLattice(path); valid)
            m_cubeWidget->drawCube(img);
}