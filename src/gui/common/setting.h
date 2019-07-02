#pragma once

#include <QtWidgets/QWidget>


class ParameterSerialList;

class SettingWidget : public QWidget
{
  public:
    SettingWidget(ParameterSerialList *settings, const QString &header = "", QWidget *parent = nullptr);

  private:
    ParameterSerialList *m_settings;
    QString m_headerName;
};