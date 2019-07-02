#pragma once

#include <utils/generic.h>
#include "parameter.h"


class ParameterList
{
public:
  ParameterList() = default;

public:
  using VecT = UPtrV<Parameter>;
  using VecIt  = VecT::iterator;
  using VecCIt = VecT::const_iterator;
  VecIt begin();
  VecIt end();
  VecCIt begin() const;
  VecCIt end() const;

public:
  template <typename T, typename... P> T* Add(P&&... params);
  bool Delete(const std::string &name);

  template <typename T> T* Get(const std::string &name);
  template <typename T> T* const Get(const std::string &name) const;

private:
  bool HasName(const std::string &name) const;

private:
  VecT m_params;
};

#include "parameterlist.hpp"