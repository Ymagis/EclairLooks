#pragma once

#include <map>

#include <QtWidgets/QWidget>

#include <utils/generic.h>
#include <core/types.h>


class LookWidget;
class ImageWidget;
class NeutralWidget;
class CubeWidget;
class QLabel;
class Image;

class LookDetailWidget : public QWidget
{
  public:
    using CompareMap = std::map<SideBySide, QString>;

  public:
    LookDetailWidget(QWidget *parent = nullptr);

  public:
    void showEvent(QShowEvent *event) override;

  public:
    void setLookWidget(LookWidget *lw);

    void resetViews();
    void clearView(SideBySide c);
    void updateView(SideBySide c);
    void showDetail(const QString &path, SideBySide c);

  private:
    LookWidget *m_lookWidget = nullptr;
    ImageWidget *m_imageWidget;
    NeutralWidget *m_neutralsWidget;
    CubeWidget *m_cubeWidget;

    QLabel *m_titleLabel;

    CompareMap m_cmap;
};