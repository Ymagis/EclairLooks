#pragma once

#include <string>
#include <vector>

#include <OpenColorIO/OpenColorIO.h>

#include <QtCore/QStringList>

#include "../imageoperator.h"


class Image;

class OCIOFileTransform : public ImageOperator
{
  public:
    OCIOFileTransform();

  public:
    ImageOperator *OpCreate() const override;
    ImageOperator *OpCreateFromPath(const std::string &filepath) const override;
    std::string OpName() const override;
    std::string OpLabel() const override;
    std::string OpDesc() const override;
    void OpApply(Image &img) override;
    bool OpIsIdentity() const override;
    void OpUpdateParamCallback(const Parameter &op) override;

  public:
    void SetFileTransform(const std::string &lutpath);

  public:
    QStringList SupportedExtensions() const;
    void OverrideInterpolation();

  private:
    OCIO_NAMESPACE::ConstConfigRcPtr m_config;
    OCIO_NAMESPACE::ConstProcessorRcPtr m_processor;
    OCIO_NAMESPACE::FileTransformRcPtr m_transform;
};