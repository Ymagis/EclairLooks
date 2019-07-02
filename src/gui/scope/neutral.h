#pragma once

#include <vector>
#include <map>

#include <QtWidgets/QGraphicsView>

#include <utils/generic.h>
#include <core/image.h>


class Image;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsItem;
class QGraphicsLineItem;
class QGraphicsTextItem;
class QGraphicsPathItem;

struct CurveItems {
    Image image;
    QGraphicsPathItem *curve[3];
    QGraphicsTextItem *curveName;
    QGraphicsLineItem *curveLegend;
    QGraphicsLineItem *cursorHLine[3];
    QGraphicsLineItem *cursorVLine;
    QGraphicsTextItem *cursorRGBValues;
    QString name;
};

class NeutralWidget : public QGraphicsView
{
  public:
    NeutralWidget(QWidget *parent = nullptr);

  public:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

  public:
    void clearView();

    void drawCurve(uint8_t id, const Image &img, const QString &path="");
    void clearCurve(uint8_t id);

  private:
    CurveItems initCurve(uint8_t id, QString path, const Image &img);

    void drawGrid();
    void drawCurve(const CurveItems &items, uint8_t id);
    void drawCursor(uint16_t x, uint16_t y);
    void hideCursors();
    void showCursors();

  private:
    UPtr<QGraphicsScene> m_scene;
    std::map<uint8_t, CurveItems> m_curves;
};