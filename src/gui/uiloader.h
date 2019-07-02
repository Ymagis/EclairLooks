#pragma once

#include <QtUiTools/QUiLoader>


class UiLoader : public QUiLoader
{
  public:
    UiLoader(QObject *parent = 0);

  public:
    QWidget *createWidget(const QString &className, QWidget *parent = 0,
                          const QString &name = QString());
};
