#include "parameter.h"
#include "widget.h"

#include <QtCore/QSettings>


FilePathParameter::FilePathParameter(const std::string &name)
    : Parameter(name), m_path_type(PathType::File)
{
}

FilePathParameter::FilePathParameter(const std::string &name, const std::string &value,
                                     const std::string &dialog_title,
                                     const std::string &filters, PathType pt)
    : Parameter(name), m_value(value), m_description(dialog_title), m_filters(filters),
      m_path_type(pt)
{
}

std::string FilePathParameter::value() const { return m_value; }

void FilePathParameter::setValue(const std::string &v)
{
    m_value = v;
    EmitEvent<UpdateValue>(*this);
}

std::string FilePathParameter::description() const { return m_description; }

void FilePathParameter::setDescription(const std::string &v)
{
    m_description = v;
    EmitEvent<UpdateSpecification>(*this);
}

std::string FilePathParameter::filters() const { return m_filters; }

void FilePathParameter::setFilters(const std::string &v)
{
    m_filters = v;
    EmitEvent<UpdateSpecification>(*this);
}

FilePathParameter::PathType FilePathParameter::pathType() const { return m_path_type; }

void FilePathParameter::setPathType(const PathType &v)
{
    m_path_type = v;
    EmitEvent<UpdateSpecification>(*this);
}

ParameterWidget *FilePathParameter::newWidget(QWidget * parent)
{
    return new ParameterFilePathWidget(this, parent);
}

void FilePathParameter::load(const QSettings *setting)
{
    setValue(setting->value(QString::fromStdString(name())).toString().toStdString());
}

void FilePathParameter::save(QSettings *setting) const
{
    setting->setValue(QString::fromStdString(name()), QString::fromStdString(value()));
}