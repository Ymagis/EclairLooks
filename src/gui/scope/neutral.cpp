#include "neutral.h"

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QtWidgets>

#include <utils/gl.h>
#include <core/image.h>


uint16_t grid_width = 640;
uint16_t grid_height = 400;

uint16_t legend_v_spacing = 35;

QColor backgroundColor = QColor(200, 200, 200);
QColor gridSmallColor = QColor(140, 140, 140);
QColor gridLargeColor = QColor(100, 100, 100);
QColor lineYColor = QColor(100, 100, 100);
QColor legendColor = QColor(50, 50, 50);

QColor lineRColorA = QColor(170, 25, 25);
QColor lineGColorA = QColor(25, 170, 25);
QColor lineBColorA = QColor(25, 25, 170);

QColor lineRColorB = QColor(200, 75, 75);
QColor lineGColorB = QColor(75, 200, 75);
QColor lineBColorB = QColor(75, 75, 200);

QColor colors_a[3] = { lineRColorA, lineGColorA, lineBColorA };
QColor colors_b[3] = { lineRColorB, lineGColorB, lineBColorB };

NeutralWidget::NeutralWidget(QWidget *parent)
: QGraphicsView(parent)
{
    m_scene = std::make_unique<QGraphicsScene>();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewport(new QOpenGLWidget());
    setBackgroundBrush(QBrush(backgroundColor));
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setScene(m_scene.get());

    drawGrid();
}

void NeutralWidget::resizeEvent(QResizeEvent *event)
{
    fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void NeutralWidget::mouseMoveEvent(QMouseEvent *event)
{
    drawCursor(event->x(), event->y());
}

void NeutralWidget::enterEvent(QEvent * event)
{
    showCursors();
    QWidget::enterEvent(event);
}

void NeutralWidget::leaveEvent(QEvent * event)
{
    hideCursors();
    QWidget::leaveEvent(event);
}

void NeutralWidget::clearView()
{
    for (auto& [k, v] : m_curves)
        clearCurve(k);

    m_curves.clear();
}

void NeutralWidget::drawCurve(uint8_t id, const Image &img, const QString &path)
{
    CurveItems curveItems;
    QFileInfo fi(path);
    QString fileName = fi.fileName();
    if (auto c = m_curves.find(id); c == m_curves.end()) {
        m_curves[id] = initCurve(id, fileName, img);
        m_curves[id];
        curveItems = m_curves[id];
    }
    else {
        m_curves[id].image = img;
        m_curves[id].name = fileName;
        curveItems = m_curves[id];
    }
    drawCurve(curveItems, id);
    fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void NeutralWidget::clearCurve(uint8_t id)
{
    if (auto c = m_curves.find(id); c != m_curves.end()) {
        CurveItems & items = c->second;

        m_scene->removeItem(items.curve[0]);
        m_scene->removeItem(items.curve[1]);
        m_scene->removeItem(items.curve[2]);
        m_scene->removeItem(items.cursorHLine[0]);
        m_scene->removeItem(items.cursorHLine[1]);
        m_scene->removeItem(items.cursorHLine[2]);
        m_scene->removeItem(items.cursorVLine);
        m_scene->removeItem(items.cursorRGBValues);
        m_scene->removeItem(items.curveName);
        m_scene->removeItem(items.curveLegend);
        m_curves.erase(id);
    }
}

void NeutralWidget::hideCursors()
{
    for (auto c = m_curves.begin(); c != m_curves.end(); ++c) {
        CurveItems &items = c->second;
        items.cursorHLine[0]->setVisible(false);
        items.cursorHLine[1]->setVisible(false);
        items.cursorHLine[2]->setVisible(false);
        items.cursorVLine->setVisible(false);
        items.cursorRGBValues->setVisible(false);
    }
}

void NeutralWidget::showCursors()
{
    for (auto &[id, items] : m_curves) {
        items.cursorHLine[0]->setVisible(true);
        items.cursorHLine[1]->setVisible(true);
        items.cursorHLine[2]->setVisible(true);
        items.cursorVLine->setVisible(true);
        items.cursorRGBValues->setVisible(true);
    }
}

CurveItems NeutralWidget::initCurve(uint8_t id, QString path, const Image &img)
{
    CurveItems items;

    QPen pen;
    pen.setWidth(2);
    pen.setStyle(id == 1 ? Qt::DashLine : Qt::SolidLine);

    // Curves & Cursor
    for (uint8_t i = 0; i < 3; ++i) {
        pen.setColor(id == 1 ? colors_b[i] : colors_a[i]);
        items.curve[i] = m_scene->addPath(QPainterPath(), pen);
        items.cursorHLine[i] = m_scene->addLine(QLineF(), pen);
    }

    items.cursorRGBValues = m_scene->addText("");
    items.curveName = m_scene->addText("");
    items.curveName->setTextWidth(grid_width / 2);
    pen.setColor(lineYColor);
    items.cursorVLine = m_scene->addLine(QLineF(), pen);
    pen.setColor(legendColor);
    items.curveLegend = m_scene->addLine(QLineF(), pen);

    items.name = path;
    items.image = img;

    return items;
}

void NeutralWidget::drawGrid()
{
    QPen pen;
    pen.setWidth(1);

    uint16_t grid_count = 5;

    // Draw Grid
    for (uint16_t i = 0; i <= grid_count; ++i) {
        pen.setColor(gridLargeColor);
        uint16_t xi = i * (1.0 * grid_width / grid_count);
        uint16_t yi = i * (1.0 * grid_height / grid_count);
        m_scene->addLine(xi, 0, xi, grid_height, pen);
        m_scene->addLine(0, yi, grid_width, yi, pen);

        if (i < grid_count) {
            pen.setColor(gridSmallColor);
            for (uint16_t j = 1; j < grid_count; ++j) {
                uint16_t xj = xi + j * (1.0 * grid_width / (grid_count * grid_count));
                uint16_t yj = yi + j * (1.0 * grid_height / (grid_count * grid_count));
                m_scene->addLine(xj, 0, xj, grid_height, pen);
                m_scene->addLine(0, yj, grid_width, yj, pen);
            }
        }
    }

    // Draw identity curve
    m_scene->addLine(0.0, grid_height, grid_width, 0.0, pen);
}

void NeutralWidget::drawCurve(const CurveItems &items, uint8_t id)
{
    // Draw R,G,B curves
    const float * pix = items.image.pixels_asfloat();
    for (uint8_t c = 0; c < 3; ++c) {
        QPainterPath path;
        path.moveTo(0, grid_height - (grid_height * pix[c]));

        for (uint32_t i = 0; i < items.image.width(); ++i) {
            path.lineTo(
                (1.0 * i / items.image.width()) * grid_width,
                grid_height - (grid_height * pix[i * 3 + c])
            );
        }

        items.curve[c]->setPath(path);
    }

    // Draw legends
    uint16_t xmid = grid_width - (grid_width / 2);
    uint16_t ypos = grid_height + 15 + (id * legend_v_spacing);
    items.curveName->setHtml(QString("<font color=\"black\">%1</font>").arg(items.name));
    items.curveName->setPos(xmid, ypos);

    if (items.name != "")
        items.curveLegend->setLine(xmid - 35, ypos + 12, xmid - 10, ypos + 12);
}

void NeutralWidget::drawCursor(uint16_t x, uint16_t y)
{
    QPointF scenePos = mapToScene(x, y);

    // Clamp at normalized 0..1 range
    float inX = scenePos.x() / grid_width;
    inX = std::clamp(inX, 0.0f, 1.0f);
    scenePos.setX(inX * grid_width);

    // Draw a cursor for each transforms
    uint8_t count = 0;
    for (auto& [id, items] : m_curves) {

        uint16_t inXInt = std::clamp((int) (inX * items.image.width()), 0, items.image.width() - 1);
        const float * pix = items.image.pixels_asfloat();

        float out[3];
        for (uint8_t i = 0; i < 3; ++i) {
            out[i] = pix[inXInt * 3 + i];
            items.cursorHLine[i]->setLine(QLineF(
                0, grid_height - (out[i] * grid_height),
                grid_width, grid_height - (out[i] * grid_height)));
        }

        items.cursorVLine->setLine(QLineF(
            scenePos.x(), 0, scenePos.x(), grid_height));

        items.cursorRGBValues->setHtml(
            QString("<font color=\"black\">%1 "
                    "=> "
                    "</font>"
                    "<font color=\"red\">%2</font> "
                    "<font color=\"green\">%3</font> "
                    "<font color=\"blue\">%4</font> ")
                .arg(QString::number(inX, 'f', 5))
                .arg(QString::number(out[0], 'f', 5))
                .arg(QString::number(out[1], 'f', 5))
                .arg(QString::number(out[2], 'f', 5)));

        items.cursorRGBValues->setPos(25, grid_height + 15 + count * legend_v_spacing);

        count++;
    }
}
