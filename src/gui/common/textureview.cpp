#include "textureview.h"

#include <cstdlib>

#include <QtCore/qmath.h>
#include <QtCore/QMimeData>
#include <QtWidgets/QMainWindow>
#include <QtGui/QGuiApplication>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QWindow>
#include <QtGui/QMatrix4x4>
#include <QtGui/QScreen>

#include <utils/generic.h>
#include <utils/gl.h>


TextureView::TextureView(QWidget *parent)
    : QOpenGLWidget(parent), m_textureRatio(1.f, 1.f)
{
    setFocusPolicy(Qt::ClickFocus);
    resetView();
}

void TextureView::mousePressEvent(QMouseEvent *event)
{
    if (QGuiApplication::keyboardModifiers() == Qt::AltModifier) {
        m_imagePosition = -widgetToWorld(event->localPos());
    }
    else if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
        setMouseTracking(true);
        m_clickPosition = widgetToNorm(event->localPos());
    }

    update();
}

void TextureView::mouseMoveEvent(QMouseEvent *event)
{
    if (QGuiApplication::keyboardModifiers() == Qt::AltModifier)
        return;
    else if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
        m_moveDelta = widgetToNorm(event->localPos()) - m_clickPosition;

    update();
}

void TextureView::mouseReleaseEvent(QMouseEvent *event)
{
    setMouseTracking(false);
    m_imagePosition += m_moveDelta;
    m_moveDelta = QPointF(0.f, 0.f);

    update();
}

void TextureView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        resetView();
    }

    update();
}

void TextureView::wheelEvent(QWheelEvent *event)
{
    float delta = 0.f;

    QPoint numPixels = event->pixelDelta();
    if (!numPixels.isNull())
        delta = numPixels.y() / 60.0;
    else
        delta = event->angleDelta().y() / 60.0;

    delta = std::clamp(delta, -0.2f, 0.2f);
    m_imageScale += delta;
    m_imageScale = std::clamp(m_imageScale, 0.1f, 25.f);

    update();
}

void TextureView::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
      case Qt::Key_Backspace:
        resetView();
        break;
      default:
        QWidget::keyPressEvent(event);
  }
}

void TextureView::initializeGL()
{
    initializeOpenGLFunctions();

    GL_CHECK(m_vao.create());
    GL_CHECK(m_vao.bind());

    GLfloat vertices[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
    };

    GL_CHECK(m_vertices.create());
    GL_CHECK(m_vertices.bind());
    GL_CHECK(m_vertices.allocate(vertices, sizeof(vertices)));
    GL_CHECK(glEnableVertexAttribArray(UnderlyingT(AttributeLocation::Position)));
    GL_CHECK(glVertexAttribPointer(
        UnderlyingT(AttributeLocation::Position), 2, GL_FLOAT, GL_FALSE, 0, 0));

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    GL_CHECK(m_colors.create());
    GL_CHECK(m_colors.bind());
    GL_CHECK(m_colors.allocate(colors, sizeof(colors)));
    GL_CHECK(glEnableVertexAttribArray(UnderlyingT(AttributeLocation::Color)));
    GL_CHECK(glVertexAttribPointer(
        UnderlyingT(AttributeLocation::Color), 3, GL_FLOAT, GL_FALSE, 0, 0));

    GLfloat texCoords[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };

    GL_CHECK(m_texCoords.create());
    GL_CHECK(m_texCoords.bind());
    GL_CHECK(m_texCoords.allocate(texCoords, sizeof(texCoords)));
    GL_CHECK(glEnableVertexAttribArray(UnderlyingT(AttributeLocation::TextureCoord)));
    GL_CHECK(glVertexAttribPointer(
        UnderlyingT(AttributeLocation::TextureCoord), 2, GL_FLOAT, GL_FALSE, 0, 0));

    GL_CHECK(m_vao.release());

    printOpenGLInfo();
}

void TextureView::resizeGL(int w, int h)
{
    const qreal retinaScale = devicePixelRatio();
    GL_CHECK(glViewport(0, 0, w * retinaScale, h * retinaScale));
}

void TextureView::setDefaultScale(float s)
{
    m_defaultScale = s;
    resetView();
}

void TextureView::setAspectRatio(float x, float y)
{
    m_textureRatio = QPointF(x, y);
}

void TextureView::setAspectRatio(float ratio)
{
    QSize dstSize  = size();
    float dstRatio = 1.0f * dstSize.width() / dstSize.height();

    if (dstRatio > ratio)
        setAspectRatio((ratio * dstSize.height()) / dstSize.width(), 1.0);
    else
        setAspectRatio(1.0, (dstSize.width() / ratio) / dstSize.height());
}

QString TextureView::defaultVertexShader() const
{
    std::string source = R"(
        #version 410 core
        layout(location = 0) in vec2 posAttr;
        layout(location = 1) in vec3 colAttr;
        layout(location = 2) in vec2 texCoordAttr;

        out vec4 col;
        out vec2 texCoord;

        uniform mat4 matrix;

        void main() {
           col = vec4(colAttr, 1.0f);
           texCoord = texCoordAttr;
           gl_Position = matrix * vec4(posAttr, 0.0f, 1.0f);
        }
    )";

    return QString::fromStdString(source);
}

QString TextureView::defaultFragmentShader() const
{
    std::string source = R"(
        #version 410 core
        in vec4 col;
        in vec2 texCoord;

        layout(location = 0) out vec4 fragColor;

        uniform sampler2D imgTex;

        void main() {
           fragColor = texture(imgTex, texCoord);
        }
    )";

    return QString::fromStdString(source);
}

void TextureView::resetView()
{
    m_imageScale = m_defaultScale;
    m_imagePosition = QPointF(0.f, 0.f);
    m_clickPosition = QPointF(0.f, 0.f);
    m_moveDelta = QPointF(0.f, 0.f);

    update();
}

QPointF TextureView::widgetToNorm(const QPointF &pos) const
{
    return QPointF(1.f * pos.x() / width(), 1.f * pos.y() / height());
}

QPointF TextureView::normToWidget(const QPointF &pos) const
{
    return QPointF(pos.x() * width(), pos.y() * height());
}

QPointF TextureView::widgetToClip(const QPointF &pos) const
{
    return widgetToNorm(pos) * 2.f - QPointF(1.f, 1.f);
}

QPointF TextureView::clipToWidget(const QPointF &pos) const
{
    return normToWidget((pos + QPointF(1.f, 1.f)) / 2.f);
}

QPointF TextureView::widgetToWorld(const QPointF &pos) const
{
    QPointF clipPoint = widgetToClip(pos);
    QVector3D clipPos(clipPoint.x(), -clipPoint.y(), 0.f);
    QVector3D worldPos = viewMatrix().inverted() * clipPos;
    return QPointF(worldPos.x(), worldPos.y());
}

QPointF TextureView::worldToWidget(const QPointF &pos) const
{
    QVector3D worldPos(pos.x(), pos.y(), 0.f);
    QVector3D clipPos = viewMatrix() * worldPos;
    return clipToWidget(QPointF(clipPos.x(), -clipPos.y()));
}

QOpenGLVertexArrayObject & TextureView::vaoObject()
{
    return m_vao;
}

QMatrix4x4 TextureView::worldMatrix() const
{
    // Warning : if not identity must update coordinates convert methods !
    return QMatrix4x4();
}

QMatrix4x4 TextureView::viewMatrix() const
{
    QMatrix4x4 view;

    view.scale(m_textureRatio.x(), m_textureRatio.y());

    // Inverse Y scale to account for OpenGL Y axis (bottom top)
    view.scale(m_imageScale, -m_imageScale);

    view.translate(
        m_imagePosition.x() + m_moveDelta.x(),
        m_imagePosition.y() + m_moveDelta.y());

    return view;
}

QMatrix4x4 TextureView::projMatrix() const
{
    // Warning : if not identity must update coordinates convert methods !
    return QMatrix4x4();
}