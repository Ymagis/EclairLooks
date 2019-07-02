#include "parameterseriallist.h"

#include <QtCore/QSettings>
#include <QtCore/QDebug>


ParameterSerialList::ParameterSerialList()
{
    m_settings = std::make_unique<QSettings>();
}

ParameterSerialList::~ParameterSerialList()
{
    SaveParameters();
}

const ParameterList & ParameterSerialList::Parameters() const
{
    return m_paramList;
}

void ParameterSerialList::LoadParameters()
{
    for (auto &p : m_paramList)
        p->load(m_settings.get());
}

void ParameterSerialList::SaveParameters()
{
    for (auto &p : m_paramList)
        p->save(m_settings.get());
}
