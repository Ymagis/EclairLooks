#pragma once

#include <string>

#include "../imageoperator.h"


class Image;

class CTLTransform : public ImageOperator
{
  public:
    CTLTransform();

  public:
    ImageOperator *OpCreate() const override;
    ImageOperator *OpCreateFromPath(const std::string &filepath) const override;
    std::string OpName() const override;
    std::string OpLabel() const override;
    void OpApply(Image &img) override;
    bool OpIsIdentity() const override;
    void OpUpdateParamCallback(const Parameter &op) override;

    void SetBase(const std::string &configpath);

  private:
    std::vector<std::string> m_searchsPath;
    std::string m_ctlFile;
};