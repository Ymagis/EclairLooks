#pragma once

#include <array>

#include "textureview.h"
#include <utils/event_source.h>
#include <core/types.h>


class Image;

typedef EventDesc <
    FuncT<void(QOpenGLTexture &tex)>,
    FuncT<void(const Image &img)>> IWEvtDesc;

class ImageWidget : public TextureView, public EventSource<IWEvtDesc>
{
  public:
    enum Evt { Update = 0, DropImage };

  public:
    ImageWidget(QWidget *parent = nullptr);

  public:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

  public:
    bool hasImage() const;
    void resetImage(const Image &img);
    void updateImage(SideBySide sbs, const Image &img);

    GLint texture();

  private:

    void createTexture(QOpenGLTexture &tex, const Image &img);
    bool guessPixelsParameters(
            const Image &img,
            QOpenGLTexture::PixelType & pt,
            QOpenGLTexture::PixelFormat &pf,
            std::array<QOpenGLTexture::SwizzleValue, 4> &sw);

  private:
    GLuint m_matrixUniform;
    GLuint m_textureUniformA;
    GLuint m_textureUniformB;
    GLuint m_sliderPosUniform;

    QOpenGLShaderProgram m_program;
    QOpenGLTexture m_textureA;
    QOpenGLTexture m_textureB;

    float m_sliderPosition;
};
