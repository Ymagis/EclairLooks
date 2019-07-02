#pragma once

#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLTexture>
#include <QtWidgets/QOpenGLWidget>


class TextureView : public QOpenGLWidget, public QOpenGLExtraFunctions
{
  public:
    TextureView(QWidget *parent = nullptr);

  public:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;

  protected:
    void setDefaultScale(float s);
    void setAspectRatio(float x, float y);
    void setAspectRatio(float ratio);

    QString defaultVertexShader() const;
    QString defaultFragmentShader() const;

    QPointF widgetToNorm(const QPointF &pos) const;
    QPointF normToWidget(const QPointF &pos) const;
    QPointF widgetToClip(const QPointF & pos) const;
    QPointF clipToWidget(const QPointF &pos) const;
    QPointF widgetToWorld(const QPointF & pos) const;
    QPointF worldToWidget(const QPointF &pos) const;

    QOpenGLVertexArrayObject & vaoObject();

    QMatrix4x4 worldMatrix() const;
    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projMatrix() const;

    void resetView();

  private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vertices;
    QOpenGLBuffer m_colors;
    QOpenGLBuffer m_texCoords;

    QPointF m_imagePosition;
    QPointF m_clickPosition;
    QPointF m_moveDelta;
    QPointF m_textureRatio;

    float m_imageScale;
    float m_defaultScale = 1.f;
};
