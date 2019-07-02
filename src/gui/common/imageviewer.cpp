#include "imageviewer.h"

#include <cstdlib>

#include <QtCore/qmath.h>
#include <QtCore/QMimeData>
#include <QtWidgets/QMainWindow>
#include <QtGui/QGuiApplication>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QWindow>
#include <QtGui/QMatrix4x4>
#include <QtGui/QScreen>
#include <QtGui/QPainter>

#include <core/image.h>
#include <utils/gl.h>
#include <utils/chrono.h>


static std::string fragmentShaderSource = R"(
    #version 410 core
    in vec4 col;
    in vec2 texCoord;

    layout(location = 0) out vec4 fragColor;

    uniform sampler2D imgTexA;
    uniform sampler2D imgTexB;
    uniform float sliderPosition;

    void main() {
       if (texCoord.x > sliderPosition)
           fragColor = texture(imgTexA, texCoord);
       else
           fragColor = texture(imgTexB, texCoord);
    }
)";


ImageWidget::ImageWidget(QWidget *parent)
    : TextureView(parent), m_textureA(QOpenGLTexture::Target2D),
      m_textureB(QOpenGLTexture::Target2D), m_sliderPosition(0.f)
{
    setAcceptDrops(true);
}

void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (QGuiApplication::keyboardModifiers() == Qt::NoModifier) {
        setMouseTracking(true);
        m_sliderPosition = widgetToWorld(event->localPos()).x();
        m_sliderPosition = std::clamp(m_sliderPosition, -1.f, 1.f);
    }

    TextureView::mousePressEvent(event);
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (QGuiApplication::keyboardModifiers() == Qt::NoModifier) {
        m_sliderPosition = widgetToWorld(event->localPos()).x();
        m_sliderPosition = std::clamp(m_sliderPosition, -1.f, 1.f);
    }

    TextureView::mouseMoveEvent(event);
}

void ImageWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void ImageWidget::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        qDebug() << "Dropped file:" << fileName << "\n";

        Image img = Image::FromFile(fileName.toStdString());
        if (img)
            EmitEvent<DropImage>(img);
    }

    update();
}

void ImageWidget::initializeGL()
{
    TextureView::initializeGL();

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, defaultVertexShader());
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource.c_str());
    m_program.link();

    GL_CHECK(m_matrixUniform = m_program.uniformLocation("matrix"));
    GL_CHECK(m_textureUniformA = m_program.uniformLocation("imgTexA"));
    GL_CHECK(m_textureUniformB = m_program.uniformLocation("imgTexB"));
    GL_CHECK(m_sliderPosUniform = m_program.uniformLocation("sliderPosition"));
}

void ImageWidget::resizeGL(int w, int h)
{
    // Aspect ratio adaptation
    float srcRatio = 1.0f * m_textureA.width() / m_textureA.height();
    setAspectRatio(srcRatio);

    TextureView::resizeGL(w, h);
}

void ImageWidget::paintGL()
{
    QPainter p(this);

    // Draw the texture(s)
    p.beginNativePainting();

        GL_CHECK(glClearColor(0.0, 0.0, 0.0, 0.0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        bool texComplete = m_textureA.isStorageAllocated() && m_textureB.isStorageAllocated();
        if (!texComplete)
            return;

        GL_CHECK(vaoObject().bind());
        GL_CHECK(m_program.bind());

        GL_CHECK(m_textureA.bind(0));
        GL_CHECK(m_textureB.bind(1));

        GL_CHECK(m_program.setUniformValue(m_matrixUniform, viewMatrix()));
        GL_CHECK(m_program.setUniformValue(m_textureUniformA, 0));
        GL_CHECK(m_program.setUniformValue(m_textureUniformB, 1));
        GL_CHECK(m_program.setUniformValue(m_sliderPosUniform, (m_sliderPosition + 1.f) / 2.f));

        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

        GL_CHECK(m_textureB.release());
        GL_CHECK(m_textureA.release());

        GL_CHECK(m_program.release());
        GL_CHECK(vaoObject().release());

    p.endNativePainting();

    // Draw the slider
    p.setPen(Qt::white);
    p.drawLine(
        worldToWidget(QPointF(m_sliderPosition, -1.f)),
        worldToWidget(QPointF(m_sliderPosition, 1.f)));
}

bool ImageWidget::hasImage() const
{
    return m_textureA.isStorageAllocated();
}

void ImageWidget::resetImage(const Image &img)
{
    makeCurrent();

    m_textureA.destroy();
    m_textureB.destroy();

    createTexture(m_textureA, img);
    createTexture(m_textureB, img);

    // Aspect ratio adaptation
    float srcRatio = 1.0f * m_textureA.width() / m_textureA.height();
    setAspectRatio(srcRatio);

    resetView();

    doneCurrent();
}

void ImageWidget::updateImage(SideBySide sbs, const Image &img)
{
    QOpenGLTexture::PixelType pixelType;
    QOpenGLTexture::PixelFormat pixelFormat;
    std::array<QOpenGLTexture::SwizzleValue, 4> sw;
    if (!guessPixelsParameters(img, pixelType, pixelFormat, sw))
        return;

    QOpenGLTexture *texture = nullptr;
    switch (sbs) {
        case SideBySide::A:
            texture = &m_textureA;
            break;
        case SideBySide::B:
            texture = &m_textureB;
            break;
    }

    if (texture->width() != img.width() || texture->height() != img.height())
        resetImage(img);

    makeCurrent();
    texture->setData(pixelFormat, pixelType, img.pixels());
    doneCurrent();

    update();

    EmitEvent<Evt::Update>(*texture);
}

GLint ImageWidget::texture()
{
    return m_textureA.textureId();
}

void ImageWidget::createTexture(QOpenGLTexture &tex, const Image &img)
{
    QOpenGLTexture::TextureFormat textureFormat = QOpenGLTexture::RGBA32F;
    QOpenGLTexture::PixelType pixelType;
    QOpenGLTexture::PixelFormat pixelFormat;
    std::array<QOpenGLTexture::SwizzleValue, 4> sw;
    if (!guessPixelsParameters(img, pixelType, pixelFormat, sw))
        return;

    tex.destroy();
    tex.setSize(img.width(), img.height());
    tex.setFormat(textureFormat);
    tex.setSwizzleMask(sw[0], sw[1], sw[2], sw[3]);
    tex.setMinificationFilter(QOpenGLTexture::Linear);
    tex.setMagnificationFilter(QOpenGLTexture::Linear);
    tex.allocateStorage();
    tex.setData(pixelFormat, pixelType, img.pixels());

    // Because swizzle mask are disabled on Qt macOS (do not know why.)
    // http://code.qt.io/cgit/qt/qtbase.git/tree/src/gui/opengl/qopengltexture.cpp?h=dev#n4009
    switch (img.format())
    {
        case PixelFormat::GRAY: {
            tex.bind();
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            tex.release();
            break;
        }
        default: {
            break;
        }
    }
}

bool ImageWidget::guessPixelsParameters(const Image &img, QOpenGLTexture::PixelType &pt,
                                        QOpenGLTexture::PixelFormat &pf,
                                        std::array<QOpenGLTexture::SwizzleValue, 4> &sw)
{
    if (!img) {
        qWarning() << "Could not load image !\n";
        return false;
    }
    else if (img.type() != PixelType::Float) {
        qWarning() << "Image pixel type not supported (must be Float) !\n";
        return false;
    }

    pt = QOpenGLTexture::Float32;
    sw = {{QOpenGLTexture::RedValue, QOpenGLTexture::GreenValue, QOpenGLTexture::BlueValue,
          QOpenGLTexture::AlphaValue}};

    switch (img.format()) {
        case PixelFormat::GRAY: {
            sw[0] = sw[1] = sw[2] = QOpenGLTexture::RedValue;
            pf                    = QOpenGLTexture::Red;
            break;
        }
        case PixelFormat::RGB: {
            pf = QOpenGLTexture::RGB;
            break;
        }
        case PixelFormat::RGBA: {
            pf = QOpenGLTexture::RGBA;
            break;
        }
        default: {
            qInfo() << "Image pixel format not supported !\n";
            return false;
        }
    }

    return true;
}
