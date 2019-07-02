#pragma once

#include <QtCore/QStringList>

#include <utils/generic.h>
#include <parameter/parameterseriallist.h>
#include <core/imagepipeline.h>
#include <operator/imageoperatorlist.h>


class Context
{
  public:
    static Context &getInstance();

  private:
    Context();

  public:
    Context(Context const &) = delete;
    void operator=(Context const &) = delete;

    ParameterSerialList& settings();
    ImagePipeline& pipeline();
    ImageOperatorList& operators();

    QStringList supportedLookExtensions();

  private:
    UPtr<ParameterSerialList> m_settings;
    UPtr<ImagePipeline> m_pipeline;
    UPtr<ImageOperatorList> m_operators;
};
