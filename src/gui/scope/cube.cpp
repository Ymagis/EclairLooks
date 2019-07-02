#include "cube.h"

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
#include <core/image.h>


static std::string vertexShaderSource = R"(
    #version 410 core
    layout(location = 0) in vec3 posAttr;
    layout(location = 1) in vec3 colAttr;

    out vec3 col;

    uniform mat4 matrix;

    void main() {
        col = colAttr;
        gl_Position = matrix * vec4(posAttr, 1.0f);
    }
)";

static std::string vertexShaderSphereSource = R"(
    #version 410 core
    layout(location = 0) in vec3 posAttr;
    layout(location = 1) in vec3 offsetAttr;
    layout(location = 2) in vec3 colAttr;

    out vec3 col;

    uniform mat4 model;
    uniform mat4 viewProj;

    void main() {
        col = colAttr;
        gl_Position = viewProj * (model * vec4(posAttr, 1.) + vec4(2. * offsetAttr, 1.));
    }
)";

static std::string fragmentShaderSource = R"(
    #version 410 core
    in vec3 col;

    layout(location = 0) out vec4 fragColor;

    void main() {
        fragColor = vec4(col, 0.75);
    }
)";

CubeWidget::CubeWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    resetView();

    // 16ms is approximatively 60 fps
    m_timerRotate.setInterval(16);

    QObject::connect(&m_timerRotate, &QTimer::timeout, [this](){
         m_rotate += m_moveDelta;
         m_moveDelta *= 0.975;

         if (m_moveDelta.manhattanLength() <= 0.01)
            m_timerRotate.stop();

         update();
    });
}

void CubeWidget::mousePressEvent(QMouseEvent *event)
{
    if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
        m_interactMode = InteractMode::Drag;
    else if (event->button() == Qt::MiddleButton)
        m_interactMode = InteractMode::Drag;
    else if (event->button() == Qt::LeftButton)
        m_interactMode = InteractMode::Rotate;

    m_timerRotate.stop();
    m_lastPosition = widgetToNorm(event->localPos());

    setMouseTracking(true);

    update();
}

void CubeWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_moveDelta = widgetToNorm(event->localPos()) - m_lastPosition;

    switch (m_interactMode) {
        case InteractMode::Rotate:
            m_rotate += m_moveDelta;
            break;
        case InteractMode::Drag:
            m_moveDelta.ry() *= -1.f;
            m_translate += m_moveDelta;
            break;
    }
    m_lastPosition = widgetToNorm(event->localPos());

    update();
}

void CubeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_interactMode == InteractMode::Rotate)
        m_timerRotate.start();

    setMouseTracking(false);

    update();
}

void CubeWidget::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton) {
        resetView();
    }

    update();
}

void CubeWidget::wheelEvent(QWheelEvent *event)
{
    float delta = 0.f;

    QPoint numPixels = event->pixelDelta();
    if (!numPixels.isNull())
        delta = numPixels.y() / 60.0;
    else
        delta = event->angleDelta().y() / 60.0;

    delta = std::clamp(delta, -0.2f, 0.2f);
    m_scale -= delta;

    update();
}

void CubeWidget::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
      case Qt::Key_Backspace:
        resetView();
        break;
      default:
        QWidget::keyPressEvent(event);
  }
}

void CubeWidget::initializeGL()
{
    initializeOpenGLFunctions();

    setupCube();
    setupSphere();
}

void CubeWidget::paintGL()
{
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GL_CHECK(glClearColor(0.15f, 0.15f, 0.15f, 1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    GL_CHECK(m_vaoCube.bind());
    GL_CHECK(m_programCube.bind());

        // Drawing cube outline...
        GL_CHECK(m_programCube.setUniformValue(m_matrixUniform, setupMVP()));

        GL_CHECK(m_indicesCubeOutline.bind());
        GL_CHECK(glDrawElements(GL_LINES, m_indicesCubeOutline.size(), GL_UNSIGNED_INT, 0));
        GL_CHECK(m_indicesCubeOutline.release());

        // Drawing axis...
        QMatrix4x4 model;
        model.scale(1.5f);
        GL_CHECK(m_programCube.setUniformValue(m_matrixUniform, setupMVP(model)));

        GL_CHECK(m_indicesAxis.bind());
        GL_CHECK(glDrawElements(GL_LINES, m_indicesAxis.size(), GL_UNSIGNED_INT, 0));
        GL_CHECK(m_indicesAxis.release());

    GL_CHECK(m_programCube.release());
    GL_CHECK(m_vaoCube.release());

    GL_CHECK(m_vaoSphere.bind());
    GL_CHECK(m_programSphere.bind());

        // Drawing lattices...
        float sphereSpacing = 3.f;
        model = QMatrix4x4();
        model.scale(1. / (sphereSpacing * m_cubeSize));
        GL_CHECK(m_programSphere.setUniformValue(m_matrixModelSphereUniform, model));
        GL_CHECK(m_programSphere.setUniformValue(m_matrixSphereUniform, setupMVP()));

        GL_CHECK(m_indicesSphere.bind());
        uint16_t primCount = m_cubeSize * m_cubeSize * m_cubeSize;
        GL_CHECK(glDrawElementsInstanced(GL_TRIANGLES, m_indicesSphere.size(), GL_UNSIGNED_INT, 0, primCount));
        GL_CHECK(m_indicesSphere.release());

    GL_CHECK(m_programSphere.release());
    GL_CHECK(m_vaoSphere.release());
}

void CubeWidget::resizeGL(int w, int h)
{
    const qreal retinaScale = devicePixelRatio();
    GL_CHECK(glViewport(0, 0, w * retinaScale, h * retinaScale));
}

void CubeWidget::resetView()
{
    m_translate = QPointF(0.f, 0.f);
    m_rotate = QPointF(0.8f, 0.2f);
    m_lastPosition = QPointF(0.f, 0.f);
    m_moveDelta = QPointF(0.f, 0.f);
    m_scale = 2.5f;

    update();
}

void CubeWidget::setDefaultScale(float s)
{
    m_defaultScale = s;
    resetView();
}

void CubeWidget::drawCube(const Image &img)
{
    makeCurrent();
    if (!context())
        return;

    std::vector<GLfloat> sphere_positions;
    uint16_t latticeCount = m_cubeSize * m_cubeSize * m_cubeSize;
    const float *pixels = img.pixels_asfloat();
    for(int i = 0; i < latticeCount; ++i) {
        sphere_positions.push_back(pixels[i*3 + 0]);
        sphere_positions.push_back(pixels[i*3 + 1]);
        sphere_positions.push_back(pixels[i*3 + 2]);
    }

    GL_CHECK(m_positionSphere.bind());
    GL_CHECK(m_positionSphere.write(0, sphere_positions.data(), sphere_positions.size() * sizeof(GLfloat)));
    GL_CHECK(m_positionSphere.release());

    doneCurrent();

    update();
}

void CubeWidget::clearCube()
{
    makeCurrent();
    if (!context())
        return;

    std::vector<GLfloat> sphere_positions;
    for (int x = 0; x < m_cubeSize; ++x) {
        float xn = x / (m_cubeSize -1.);
        for (int y = 0; y < m_cubeSize; ++y) {
            float yn = y / (m_cubeSize -1.);
            for (int z = 0; z < m_cubeSize; ++z) {
                float zn = z / (m_cubeSize -1.);

                sphere_positions.push_back(xn);
                sphere_positions.push_back(yn);
                sphere_positions.push_back(zn);
            }
        }
    }

    GL_CHECK(m_positionSphere.bind());
    GL_CHECK(m_positionSphere.write(0, sphere_positions.data(), sphere_positions.size() * sizeof(GLfloat)));
    GL_CHECK(m_positionSphere.release());

    doneCurrent();

    update();
}

void CubeWidget::setupCube()
{
    GL_CHECK(m_vaoCube.create());
    GL_CHECK(m_vaoCube.bind());

    GLfloat cube_vertices[] = {
        0.f, 0.f, 0.f, // 1st square
        1.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f, // 2nd square
        1.f, 0.f, 1.f,
        1.f, 1.f, 1.f,
        0.f, 1.f, 1.f
    };

    GL_CHECK(m_verticesCube.create());
    GL_CHECK(m_verticesCube.bind());
    GL_CHECK(m_verticesCube.allocate(cube_vertices, sizeof(cube_vertices)));
    GL_CHECK(glEnableVertexAttribArray(UnderlyingT(AttributeLocation::Position)));
    GL_CHECK(glVertexAttribPointer(UnderlyingT(AttributeLocation::Position), 3, GL_FLOAT, GL_FALSE, 0, 0));

    GLfloat cube_colors[] = {
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f,
        1.f, 0.f, 1.f,
        1.f, 1.f, 1.f,
        0.f, 1.f, 1.f
    };

    GL_CHECK(m_colorsCube.create());
    GL_CHECK(m_colorsCube.bind());
    GL_CHECK(m_colorsCube.allocate(cube_colors, sizeof(cube_colors)));
    GL_CHECK(glEnableVertexAttribArray(UnderlyingT(AttributeLocation::Color)));
    GL_CHECK(glVertexAttribPointer(UnderlyingT(AttributeLocation::Color), 3, GL_FLOAT, GL_FALSE, 0, 0));

    GLuint cube_indexes[] = {
        0, 1, 5, // Bottom
        5, 4, 0,
        0, 1, 2, // Front
        2, 3, 0,
        1, 5, 6, // Right
        6, 2, 1,
        4, 5, 6, // Back
        6, 7, 4,
        0, 4, 7, // Left
        7, 3, 0,
        3, 2, 6, // Top
        6, 7, 3
    };

    GL_CHECK(m_indicesCube = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer));
    GL_CHECK(m_indicesCube.create());
    GL_CHECK(m_indicesCube.bind());
    GL_CHECK(m_indicesCube.allocate(cube_indexes, sizeof(cube_indexes)));

    GLuint cube_outline_indexes[] = {
        0, 1,
        1, 5,
        5, 4,
        4, 0,
        0, 3,
        1, 2,
        5, 6,
        4, 7,
        3, 2,
        2, 6,
        6, 7,
        7, 3
    };

    GL_CHECK(m_indicesCubeOutline = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer));
    GL_CHECK(m_indicesCubeOutline.create());
    GL_CHECK(m_indicesCubeOutline.bind());
    GL_CHECK(m_indicesCubeOutline.allocate(cube_outline_indexes, sizeof(cube_outline_indexes)));

    GLuint axis_indices[] = {
        0, 1,
        0, 3,
        0, 4
    };

    GL_CHECK(m_indicesAxis = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer));
    GL_CHECK(m_indicesAxis.create());
    GL_CHECK(m_indicesAxis.bind());
    GL_CHECK(m_indicesAxis.allocate(axis_indices, sizeof(axis_indices)));

    GL_CHECK(m_vaoCube.release());

    m_programCube.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource.c_str());
    m_programCube.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource.c_str());
    m_programCube.link();
    if (!m_programCube.isLinked())
        qWarning() << m_programCube.log() << "\n";

    GL_CHECK(m_matrixUniform = m_programCube.uniformLocation("matrix"));
}

void CubeWidget::setupSphere()
{
    GL_CHECK(m_vaoSphere.create());
    GL_CHECK(m_vaoSphere.bind());

    uint16_t M = 15;
    uint16_t N = 15;
    const float Pi = 3.14159265359;
    std::vector<GLfloat> sphere_vertices;
    for (int m = 0; m < M; ++m) {
        for (int n = 0; n < N; ++n) {
            float mf = (1.f * m) / (M-1);
            float nf = (1.f * n) / (N-1);
            // X - from [0..1..0] (sin(Pi . mf)), then vary inner X [1..0..-1..0..1] cos(2Pi . nf)
            sphere_vertices.push_back(sin(Pi * mf) * cos(2*Pi * nf));
            // Y - from [0..1..0] (sin(Pi . mf)), then vary inner Y [0..1..0..-1..0] sin(2Pi . nf)
            sphere_vertices.push_back(sin(Pi * mf) * sin(2*Pi * nf));
            // Z range from 1 to -1
            sphere_vertices.push_back(cos(Pi * mf));
        }
    }

    GL_CHECK(m_verticesSphere.create());
    GL_CHECK(m_verticesSphere.bind());
    GL_CHECK(m_verticesSphere.allocate(sphere_vertices.data(), sphere_vertices.size() * sizeof(GLfloat)));
    GL_CHECK(glEnableVertexAttribArray(UnderlyingT(AttributeLocation::Position)));
    GL_CHECK(glVertexAttribPointer(
        UnderlyingT(AttributeLocation::Position), 3, GL_FLOAT, GL_FALSE, 0, 0));

    std::vector<GLuint> sphere_indexes;
    for (int m = 0; m < M - 1; ++m) {
        for (int n = 0; n < N; ++n) {
            sphere_indexes.push_back(m*N + n);
            sphere_indexes.push_back((m+1)*N + n%N);
            sphere_indexes.push_back((m+1)*N + (n+1)%N);

            sphere_indexes.push_back(m*N + n);
            sphere_indexes.push_back(m*N + (n+1)%N);
            sphere_indexes.push_back((m+1)*N + (n+1)%N);
        }
    }

    GL_CHECK(m_indicesSphere = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer));
    GL_CHECK(m_indicesSphere.create());
    GL_CHECK(m_indicesSphere.bind());
    GL_CHECK(m_indicesSphere.allocate(sphere_indexes.data(), sphere_indexes.size() * sizeof(GLuint)));

    std::vector<GLfloat> sphere_positions;
    std::vector<GLfloat> sphere_colors;
    for (int x = 0; x < m_cubeSize; ++x) {
        float xn = x / (m_cubeSize -1.);
        for (int y = 0; y < m_cubeSize; ++y) {
            float yn = y / (m_cubeSize -1.);
            for (int z = 0; z < m_cubeSize; ++z) {
                float zn = z / (m_cubeSize -1.);

                sphere_positions.push_back(zn);
                sphere_positions.push_back(yn);
                sphere_positions.push_back(xn);

                sphere_colors.push_back(zn);
                sphere_colors.push_back(yn);
                sphere_colors.push_back(xn);
            }
        }
    }

    GL_CHECK(m_positionSphere.create());
    GL_CHECK(m_positionSphere.bind());
    GL_CHECK(m_positionSphere.allocate(sphere_positions.data(), sphere_positions.size() * sizeof(GLfloat)));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
    GL_CHECK(glVertexAttribDivisor(1, 1));

    GL_CHECK(m_colorSphere.create());
    GL_CHECK(m_colorSphere.bind());
    GL_CHECK(m_colorSphere.allocate(sphere_colors.data(), sphere_colors.size() * sizeof(GLfloat)));
    GL_CHECK(glEnableVertexAttribArray(2));
    GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0));
    GL_CHECK(glVertexAttribDivisor(2, 1));

    GL_CHECK(m_vaoSphere.release());

    m_programSphere.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSphereSource.c_str());
    m_programSphere.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource.c_str());
    m_programSphere.link();
    if (!m_programSphere.isLinked())
        qWarning() << m_programSphere.log() << "\n";

    GL_CHECK(m_matrixSphereUniform = m_programSphere.uniformLocation("viewProj"));
    GL_CHECK(m_matrixModelSphereUniform = m_programSphere.uniformLocation("model"));
}

QMatrix4x4 CubeWidget::setupMVP(const QMatrix4x4 &m) const
{
    // 1. Model
    QMatrix4x4 model;
    model.translate(-0.5f, -0.5f, -0.5f);
    model *= m;

    // 2. View
    QVector3D pos = QVector3D(
        cos(m_rotate.x()) * cos(m_rotate.y()),
        sin(m_rotate.y()),
        sin(m_rotate.x()) * cos(m_rotate.y()));

    QMatrix4x4 view;
    view.translate(m_translate.x(), m_translate.y());
    view.lookAt(m_scale * pos, QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    // 3. Projection
    float ratio = 1.f * width() / height();
    QMatrix4x4 projection;
    projection.perspective(45, ratio, 1., -1.);

    return projection * view * model;
}

QPointF CubeWidget::widgetToNorm(const QPointF & pos) const
{
    return QPointF(1.f * pos.x() / width(), 1.f * pos.y() / height());
}

QPointF CubeWidget::widgetToClip(const QPointF & pos) const
{
    return widgetToNorm(pos) * 2.f - QPointF(1.f, 1.f);
}
