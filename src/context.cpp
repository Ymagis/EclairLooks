#include "context.h"

#include <operator/ocio/filetransform.h>


Context& Context::getInstance()
{
    static Context instance;
    return instance;
}

Context::Context() :
    m_settings(new ParameterSerialList()),
    m_pipeline(new ImagePipeline()),
    m_operators(new ImageOperatorList())
{

}

ParameterSerialList& Context::settings() { return *m_settings; }

ImagePipeline& Context::pipeline() { return *m_pipeline; }

ImageOperatorList& Context::operators() { return *m_operators; }

QStringList Context::supportedLookExtensions()
{
    QStringList res;
    for (QString &ext : OCIOFileTransform().SupportedExtensions())
        res << "*." + ext;

    return res;
}