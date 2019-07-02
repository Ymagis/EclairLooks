#pragma once

#include <string>

#include <OpenColorIO/OpenColorIO.h>

#include "../imageoperator.h"


class Image;

class OCIOColorSpace : public ImageOperator
{
  public:
    OCIOColorSpace();

  public:
    ImageOperator *OpCreate() const override;
    ImageOperator *OpCreateFromPath(const std::string &filepath) const override;
    std::string OpName() const override;
    std::string OpLabel() const override;
    std::string OpDesc() const override;
    void OpApply(Image &img) override;
    bool OpIsIdentity() const override;
    void OpUpdateParamCallback(const Parameter &op) override;

    void SetConfig(const std::string &configpath);

  private:
    OCIO_NAMESPACE::ConstConfigRcPtr m_config;
    OCIO_NAMESPACE::ConstProcessorRcPtr m_processor;
    OCIO_NAMESPACE::LookTransformRcPtr m_transform;
};