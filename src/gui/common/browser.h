#pragma once

#include <utils/event_source.h>

#include <QtWidgets/QTreeView>


class BrowserModel;
class BrowserListWidget;
class QSortFilterProxyModel;
class QLineEdit;

typedef EventDesc <
    FuncT<void(const QString &)>> LBEvtDesc;

class BrowserWidget : public QWidget, public EventSource<LBEvtDesc>
{
  public:
    enum Evt { Select = 0 };

  public:
    BrowserWidget(QWidget *parent = nullptr);

  public:
    void setRootPath(const QString &path);
    void setExtFilter(const QStringList &filter);

  private:
    void updateSelection(const QModelIndex &index);

  private:
    QLineEdit *m_leSearch;
    BrowserListWidget *m_browser;
};

class BrowserListWidget : public QTreeView
{
  public:
    BrowserListWidget(QWidget *parent = nullptr);

  public:
    void setRootPath(const QString &path);
    void setExtFilter(const QStringList &filter);

    void filterList(const QString &filter);

  private:
    BrowserModel *m_fileSystemModel;
    QSortFilterProxyModel *m_sortFilterModel;

    QString m_rootPath;
    QStringList m_extFilter;
};
