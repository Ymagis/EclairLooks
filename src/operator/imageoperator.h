#pragma once

#include <string>
#include <vector>
#include <map>

#include <utils/event_source.h>
#include <parameter/parameterlist.h>


typedef EventDesc<
    FuncT<void(const Parameter &op)>,
    FuncT<void()>> IOPEvtDesc;

class Image;

class ImageOperator : public EventSource<IOPEvtDesc>
{
  public:
    using CategoryMapT = std::map<std::string, std::vector<std::string>>;

    // When a parameter get updated (either it's value or specification),
    // <UpdateParam> is emitted first and will typically update the operator
    // processing state, then <Update> gets called and will
    enum Evt { UpdateParam, Update };

  public:
    ImageOperator();
    virtual ~ImageOperator() = default;

  public:
    virtual ImageOperator * OpCreate() const = 0;
    virtual ImageOperator * OpCreateFromPath(const std::string &filepath) const { return nullptr; }
    virtual std::string OpName() const = 0;
    virtual std::string OpLabel() const = 0;
    virtual std::string OpDesc() const { return ""; }
    virtual void OpApply(Image &img) = 0;
    virtual bool OpIsIdentity() const { return true; }
    virtual void OpUpdateParamCallback(const Parameter &op) {}

  public:
    bool IsIdentity() const;
    void Apply(Image &img);

  public:
    template <typename T, typename... P> T* AddParameter(P&&... p);
    template <typename T, typename... P> T* AddParameterByCategory(const std::string & c, P&&... p);

    ParameterList & Parameters();
    ParameterList const & Parameters() const;

    template <typename T> T* GetParameter(const std::string &name);
    template <typename T> T* const GetParameter(const std::string &name) const;

    std::string DefaultCategory() const;
    CategoryMapT const & Categories() const;

  private:
    void UpdatedParameter(const Parameter &p);

  private:
    ParameterList m_paramList;

    CategoryMapT m_categoryMap;
    std::string m_defaultCategory = "Global";
};

#include "imageoperator.hpp"