#pragma once


template <typename T, typename... P>
T* ImageOperator::AddParameter(P&&... p)
{
    return AddParameterByCategory<T>(m_defaultCategory, std::forward<P>(p)...);
}

template <typename T, typename... P>
T* ImageOperator::AddParameterByCategory(const std::string & c, P&&... p)
{
    using std::placeholders::_1;

    if (T* param = m_paramList.Add<T>(std::forward<P>(p)...)) {
        // template keyword helps dependent name lookup, if not used compiler
        // might assume Subscribe is a variable and use operator less than.
        param->template Subscribe<Parameter::UpdateValue>(std::bind(&ImageOperator::UpdatedParameter, this, _1));
        param->template Subscribe<Parameter::UpdateSpecification>(std::bind(&ImageOperator::UpdatedParameter, this, _1));
        m_categoryMap[c].push_back(param->name());

        return param;
    }

    return nullptr;
}

template <typename T>
T* ImageOperator::GetParameter(const std::string &name)
{
    return m_paramList.Get<T>(name);
}

template <typename T>
T* const ImageOperator::GetParameter(const std::string &name) const
{
    return m_paramList.Get<T>(name);
}