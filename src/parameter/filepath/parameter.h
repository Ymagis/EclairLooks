#pragma once

#include <string>

#include "../parameter.h"


class FilePathParameter : public Parameter
{
  public:
    enum class PathType {
        File,
        Folder
    };

  public:
    FilePathParameter() = default;
    FilePathParameter(const std::string &name);
    FilePathParameter(const std::string &name, const std::string &value,
                      const std::string &dialog_title = "",
                      const std::string &filters = "", PathType pt = PathType::File);

  public:
    std::string value() const;
    void setValue(const std::string &v);

    std::string description() const;
    void setDescription(const std::string &v);

    std::string filters() const;
    void setFilters(const std::string &v);

    PathType pathType() const;
    void setPathType(const PathType &v);

  public:
    ParameterWidget *newWidget(QWidget * parent = nullptr) override;

    void load(const QSettings *setting) override;
    void save(QSettings *setting) const override;

  private:
    std::string m_value;
    std::string m_description;
    std::string m_filters;
    PathType m_path_type;
};
