#pragma once

#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLBuffer>

#include <gui/common/textureview.h>


class Image;

class VectorScopeWidget : public TextureView
{
  public:
    VectorScopeWidget(QWidget *parent = nullptr);

  public:
    void keyPressEvent(QKeyEvent *event) override;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void updateTexture(GLint tex);

  private:
    void initLegend();
    void initScope();

    void drawGraph(const QMatrix4x4 &m);

  private:
    float m_alpha;

    GLint m_textureId = -1;
    QSize m_textureSize;

    QOpenGLShaderProgram m_programScope;
    QOpenGLVertexArrayObject m_vaoScope;
    GLuint m_scopeAlphaUniform;
    GLuint m_scopeMatrixUniform;
    GLuint m_scopeTextureUniform;
    GLuint m_scopeResolutionWUniform;
    GLuint m_scopeResolutionHUniform;

    QOpenGLShaderProgram m_programLegend;
    QOpenGLVertexArrayObject m_vaoLegend;
    QOpenGLBuffer m_verticesLegend;
    GLuint m_legendAlphaUniform;
    GLuint m_legendMatrixUniform;
};