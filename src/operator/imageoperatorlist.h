#pragma once

#include <string>
#include <vector>
#include <map>

#include "../utils/generic.h"
#include "imageoperator.h"


class ImageOperatorList
{
    using OpKeyT = std::string;

  public:
    ImageOperatorList() = default;

  public:
    std::vector<OpKeyT> Operators() const;

    template <typename T> bool Register();

    ImageOperator * CreateFromName(const std::string &name);
    ImageOperator * CreateFromPath(const std::string &filepath);

  private:
    bool HasName(const std::string &name) const;

  private:
    std::map<OpKeyT, UPtr<ImageOperator>> m_ops;
};

template <typename T>
bool ImageOperatorList::Register()
{
    UPtr<ImageOperator> op(new T);

    if (HasName(op->OpName()))
        return false;

    auto [it, inserted] = m_ops.emplace(op->OpName(), std::move(op));
    return inserted;
}