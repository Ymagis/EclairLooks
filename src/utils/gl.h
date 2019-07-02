#pragma once

#include <QtCore/QDebug>
#include <QtGui/QOpenGLExtraFunctions>


// ----------------------------------------------------------------------------

#define GL_CHECK(stmt)                                                                   \
    stmt;                                                                                \
    checkOpenGLError(#stmt, __FILE__, __LINE__);

// ----------------------------------------------------------------------------

inline void checkOpenGLError(const std::string &stmt, const std::string &file, int line)
{
    QOpenGLExtraFunctions glFuncs(QOpenGLContext::currentContext());
    GLenum err;

    while ((err = glFuncs.glGetError()) != GL_NO_ERROR) {
        QString errorType;
        switch (err) {
            case GL_INVALID_OPERATION:
                errorType = "INVALID_OPERATION";
                break;
            case GL_INVALID_ENUM:
                errorType = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errorType = "INVALID_VALUE";
                break;
            case GL_OUT_OF_MEMORY:
                errorType = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorType = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }

        qCritical() << "OpenGL error" << errorType << "(" << err << ") at "
                    << QString::fromStdString(file) << ":" << line << " for "
                    << QString::fromStdString(stmt) << "\n";
    }
}

inline void printOpenGLInfo()
{
    static bool isPrinted = false;
    if (isPrinted)
        return;

    QOpenGLExtraFunctions glFuncs(QOpenGLContext::currentContext());
    auto gl_vendor   = QString(reinterpret_cast<char const *>(glFuncs.glGetString(GL_VENDOR)));
    auto gl_renderer = QString(reinterpret_cast<char const *>(glFuncs.glGetString(GL_RENDERER)));
    auto gl_version  = QString(reinterpret_cast<char const *>(glFuncs.glGetString(GL_VERSION)));
    auto gl_glsl_version =
        QString(reinterpret_cast<char const *>(glFuncs.glGetString(GL_SHADING_LANGUAGE_VERSION)));

    qInfo() << "OpenGL Context :\n"
            << "\tVendor : " << gl_vendor << "\n"
            << "\tRenderer : " << gl_renderer << "\n"
            << "\tVersion : " << gl_version << "\n"
            << "\tGLSL Version : " << gl_glsl_version << "\n";

    isPrinted = true;
}

// ----------------------------------------------------------------------------

enum class AttributeLocation : uint32_t { Position = 0, Color, TextureCoord };
