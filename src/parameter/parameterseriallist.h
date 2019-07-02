#pragma once

#include <utils/generic.h>
#include "parameterlist.h"


class QSettings;

class ParameterSerialList
{
  public:
    ParameterSerialList();
    ~ParameterSerialList();

  public:
    const ParameterList & Parameters() const;

    template <typename T, typename... P> T* Add(P&&... params);
    template <typename T> T* const Get(const std::string &name) const;

  private:
    void LoadParameters();
    void SaveParameters();

  private:
    UPtr<QSettings> m_settings;
    ParameterList m_paramList;
};

template <typename T, typename... P>
T* ParameterSerialList::Add(P&&... p)
{
    using std::placeholders::_1;

    T* param = m_paramList.Add<T>(std::forward<P>(p)...);
    param->template Subscribe<Parameter::UpdateValue>(std::bind(&Parameter::save, param, m_settings.get()));
    param->load(m_settings.get());

    return param;
}

template <typename T>
T* const ParameterSerialList::Get(const std::string &name) const
{
    return m_paramList.Get<T>(name);
}