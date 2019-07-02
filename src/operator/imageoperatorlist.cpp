#include "imageoperatorlist.h"
#include "imageoperator.h"


std::vector<ImageOperatorList::OpKeyT> ImageOperatorList::Operators() const
{
    std::vector<OpKeyT> alls;
    for (auto &[k, v] : m_ops)
        alls.push_back(k);

    return alls;
}

ImageOperator * ImageOperatorList::CreateFromName(const std::string &name)
{
    for (auto &[k, v] : m_ops)
        if (k == name)
            return v->OpCreate();

    return nullptr;
}

ImageOperator * ImageOperatorList::CreateFromPath(const std::string &filepath)
{
    for (auto &[k, v] : m_ops)
        if (ImageOperator * op = v->OpCreateFromPath(filepath))
            return op;

    return nullptr;
}

bool ImageOperatorList::HasName(const std::string &name) const
{
    for (auto &[k, v] : m_ops)
        if (k == name)
            return true;

    return false;
}