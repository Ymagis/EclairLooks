#pragma once

#include <QtCore/QTimer>
#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLTexture>
#include <QtWidgets/QOpenGLWidget>


class Image;

class CubeWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
  public:
    CubeWidget(QWidget *parent = nullptr);

  public:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

  public:
    void resetView();
    void setDefaultScale(float s);
    void drawCube(const Image &img);
    void clearCube();

  private:
    void setupCube();
    void setupSphere();
    QMatrix4x4 setupMVP(const QMatrix4x4 &model = QMatrix4x4()) const;

    QPointF widgetToNorm(const QPointF &pos) const;
    QPointF widgetToClip(const QPointF & pos) const;

  private:
    QOpenGLVertexArrayObject m_vaoCube;
    QOpenGLBuffer m_verticesCube;
    QOpenGLBuffer m_colorsCube;
    QOpenGLBuffer m_indicesCube;
    QOpenGLBuffer m_indicesCubeOutline;
    QOpenGLBuffer m_indicesAxis;
    QOpenGLShaderProgram m_programCube;
    GLuint m_matrixUniform;

    QOpenGLVertexArrayObject m_vaoSphere;
    QOpenGLBuffer m_verticesSphere;
    QOpenGLBuffer m_indicesSphere;
    QOpenGLBuffer m_positionSphere;
    QOpenGLBuffer m_colorSphere;
    QOpenGLShaderProgram m_programSphere;
    GLuint m_matrixSphereUniform;
    GLuint m_matrixModelSphereUniform;

    enum class InteractMode { Rotate, Drag };
    InteractMode m_interactMode;

    float m_defaultScale = 1.f;
    float m_scale = 1.f;
    QPointF m_translate;
    QPointF m_rotate;
    QPointF m_lastPosition;
    QPointF m_moveDelta;
    QTimer m_timerRotate;

    uint16_t m_cubeSize = 17;
};
