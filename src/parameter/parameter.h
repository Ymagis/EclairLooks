#pragma once

#include <string>
#include <vector>

#include <utils/generic.h>
#include <utils/event_source.h>


class Parameter ;
class ParameterWidget;
class QWidget;
class QSettings;

typedef EventDesc<
    FuncT<void(const Parameter &p)>,
    FuncT<void(const Parameter &p)>> PEvtDesc;

class Parameter : public EventSource<PEvtDesc>
{
  public:
    enum Evt { UpdateValue, UpdateSpecification };

  public:
    Parameter() = default;
    Parameter(const std::string &name);
    virtual ~Parameter() = default;

  public:
    operator bool() const;

  public:
    std::string name() const;

    std::string displayName() const;
    void setDisplayName(const std::string &v);

  public:
    virtual ParameterWidget *createWidget(QWidget * parent = nullptr);

    virtual void load(const QSettings* setting) = 0;
    virtual void save(QSettings* setting) const = 0;

  protected:
    virtual ParameterWidget *newWidget(QWidget* parent) = 0;

  private:
    std::string m_name;
    std::string m_display_name;
};

#include "checkbox/parameter.h"
#include "filepath/parameter.h"
#include "matrix/parameter.h"
#include "select/parameter.h"
#include "slider/parameter.h"
#include "text/parameter.h"