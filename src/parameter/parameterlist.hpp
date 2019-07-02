#pragma once


template <typename T, typename... P>
T* ParameterList::Add(P&&... p)
{
    auto param = UPtr<T>(new T(std::forward<P>(p)...));
    if (HasName(param->name()))
        return nullptr;

    m_params.push_back(std::move(param));
    return static_cast<T*>(m_params.back().get());
}

template <typename T>
T* ParameterList::Get(const std::string &name)
{
    for (auto &p : m_params)
        if (p->name() == name)
            return static_cast<T*>(p.get());

    return nullptr;
}

template <typename T>
T* const ParameterList::Get(const std::string &name) const
{
    for (auto &p : m_params)
        if (p->name() == name)
            return static_cast<T*>(p.get());

    return nullptr;
}
