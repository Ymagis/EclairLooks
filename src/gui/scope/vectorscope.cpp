#include "vectorscope.h"

#include <cassert>
#include <cmath>

#include <QtCore/QDebug>
#include <QtGui/QKeyEvent>

#include <core/image.h>
#include <utils/generic.h>
#include <utils/gl.h>


static std::string vertexShaderSource = R"(
    #version 410 core
    out vec3 color;

    uniform mat4 matrix;
    uniform sampler2D v_tex;
    uniform int width;
    uniform int height;

    void main() {
        int x = gl_VertexID % width;
        int y = gl_VertexID / width;
        vec2 pix = vec2(x, y) / vec2(width, height);

        vec4 col = 255.0 * texture(v_tex, vec2(pix.x, 1.0f - pix.y));

        float Y = (col.r * 0.299) + (col.g * 0.587) + (col.b * 0.114);
        float Cr = ((col.r - Y) * 0.713) + 128;
        float Cb = ((col.b - Y) * 0.564) + 128;

        Cr /= 255.0;
        Cb /= 255.0;

        gl_Position = vec4(Cb, Cr, 0.0, 1.0);
        gl_Position.xy = (gl_Position.xy * 2.0) - 1.;
        gl_Position.y *= -1.;
        gl_Position = matrix * gl_Position;

        color = vec3(1.0, 1.0, 1.0);
    }
)";

static std::string legendVertexShaderSource = R"(
    #version 410 core
    in vec3 colAttr;
    out vec3 color;

    uniform mat4 matrix;

    void main() {

        vec3 col = 255.0 * colAttr;

        float Y = (col.r * 0.299) + (col.g * 0.587) + (col.b * 0.114);
        float Cr = ((col.r - Y) * 0.713) + 128;
        float Cb = ((col.b - Y) * 0.564) + 128;

        Cr /= 255.0;
        Cb /= 255.0;

        gl_Position = vec4(Cb, Cr, 0.0, 1.0);
        gl_Position.xy = (gl_Position.xy * 2.0) - 1.;
        gl_Position.y *= -1.;
        gl_Position = matrix * gl_Position;

        color = colAttr;
    }
)";

static std::string fragmentShaderSource = R"(
    #version 410 core
    in vec3 color;
    layout(location = 0) out vec4 fragColor;

    uniform float alpha;

    void main() {
       fragColor = vec4(color.rgb, alpha);
       fragColor.rgb = pow(fragColor.rgb, vec3(1./2.4));
    }
)";

VectorScopeWidget::VectorScopeWidget(QWidget *parent)
    : TextureView(parent), m_alpha(0.1f)
{

}

void VectorScopeWidget::resizeGL(int w, int h)
{
    setAspectRatio(1.0f);

    TextureView::resizeGL(w, h);
}

void VectorScopeWidget::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
      case Qt::Key_Plus:
        m_alpha *= 1.2f;
        m_alpha = std::clamp(m_alpha, 0.001f, 1.0f);
        update();
        break;
      case Qt::Key_Minus:
        m_alpha *= 0.8f;
        m_alpha = std::clamp(m_alpha, 0.001f, 1.0f);
        update();
        break;
      default:
        QWidget::keyPressEvent(event);
  }

  TextureView::keyPressEvent(event);
}

void VectorScopeWidget::initializeGL()
{
    initializeOpenGLFunctions();

    initLegend();
    initScope();
}

void VectorScopeWidget::paintGL()
{
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    drawGraph(viewMatrix());
}

void VectorScopeWidget::updateTexture(GLint tex)
{
    makeCurrent();

    m_textureId = tex;

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_textureId));
    GL_CHECK(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_textureSize.rwidth()));
    GL_CHECK(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_textureSize.rheight()));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    doneCurrent();

    update();
}

void VectorScopeWidget::initLegend()
{
    m_programLegend.removeAllShaders();
    m_programLegend.addShaderFromSourceCode(QOpenGLShader::Vertex, legendVertexShaderSource.c_str());
    m_programLegend.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource.c_str());
    m_programLegend.link();
    if (!m_programLegend.isLinked())
        qWarning() << m_programLegend.log() << "\n";

    GL_CHECK(m_legendMatrixUniform = m_programLegend.uniformLocation("matrix"));
    GL_CHECK(m_legendAlphaUniform = m_programLegend.uniformLocation("alpha"));

    GL_CHECK(m_vaoLegend.destroy());
    GL_CHECK(m_vaoLegend.create());
    GL_CHECK(m_vaoLegend.bind());

    // R, G, B values for 75% and 100% signals
    std::vector<GLfloat> vertices = {
        0.75f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.75f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.75f,
        0.0f, 0.0f, 1.0f,
        0.75f, 0.75f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.75f, 0.75f,
        0.0f, 1.0f, 1.0f,
        0.75f, 0.0f, 0.75f,
        1.0f, 0.0f, 1.0f,
    };

    GL_CHECK(m_verticesLegend.destroy());
    GL_CHECK(m_verticesLegend.create());
    GL_CHECK(m_verticesLegend.bind());
    GL_CHECK(m_verticesLegend.allocate(vertices.data(), vertices.size() * sizeof(GLfloat)));
    GL_CHECK(glEnableVertexAttribArray(UnderlyingT(AttributeLocation::Position)));
    GL_CHECK(glVertexAttribPointer(
        UnderlyingT(AttributeLocation::Position), 3, GL_FLOAT, GL_FALSE, 0, 0));

    GL_CHECK(m_vaoLegend.release());
}

void VectorScopeWidget::initScope()
{
    m_programScope.removeAllShaders();
    m_programScope.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource.c_str());
    m_programScope.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource.c_str());
    m_programScope.link();
    if (!m_programScope.isLinked())
        qWarning() << m_programScope.log() << "\n";

    GL_CHECK(m_scopeTextureUniform = m_programScope.uniformLocation("v_tex"));
    GL_CHECK(m_scopeAlphaUniform = m_programScope.uniformLocation("alpha"));
    GL_CHECK(m_scopeMatrixUniform = m_programScope.uniformLocation("matrix"));
    GL_CHECK(m_scopeResolutionWUniform = m_programScope.uniformLocation("width"));
    GL_CHECK(m_scopeResolutionHUniform = m_programScope.uniformLocation("height"));

    GL_CHECK(m_vaoScope.destroy());
    GL_CHECK(m_vaoScope.create());
}

void VectorScopeWidget::drawGraph(const QMatrix4x4 &m)
{
    // Draw legend
    GL_CHECK(m_vaoLegend.bind());
    GL_CHECK(m_programLegend.bind());

        GL_CHECK(m_programLegend.setUniformValue(m_legendMatrixUniform, m));
        GL_CHECK(m_programScope.setUniformValue(m_legendAlphaUniform, 1.0f));
        GL_CHECK(glDrawArrays(GL_LINES, 0, 12));

    GL_CHECK(m_programLegend.release());
    GL_CHECK(m_vaoLegend.release());

    // Fill in waveform
    if (m_textureId == -1)
        return;

    float alpha = m_alpha;
    alpha /= 3.f;

    GL_CHECK(m_vaoScope.bind());
    GL_CHECK(m_programScope.bind());
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_textureId));

    // Turn off any filtering that could produce colors not in the original
    // image, this is needed because we access the texture using normalized
    // coordinates. Then restore originals parameters.
    GLint minFilter, magFilter;
    GL_CHECK(glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter));
    GL_CHECK(glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &magFilter));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

        GL_CHECK(m_programScope.setUniformValue(m_scopeAlphaUniform, alpha));
        GL_CHECK(m_programScope.setUniformValue(m_scopeMatrixUniform, m));
        GL_CHECK(m_programScope.setUniformValue(m_scopeResolutionWUniform, m_textureSize.width()));
        GL_CHECK(m_programScope.setUniformValue(m_scopeResolutionHUniform, m_textureSize.height()));
        GL_CHECK(glDrawArrays(GL_POINTS, 0, m_textureSize.width() * m_textureSize.height()));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CHECK(m_programScope.release());
    GL_CHECK(m_vaoScope.release());
}
