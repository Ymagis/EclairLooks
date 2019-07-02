#pragma once

#include <QtWidgets/QTabWidget>

#include <utils/generic.h>
#include <utils/event_source.h>


typedef EventDesc <
    FuncT<void(const QString &)>,
    FuncT<void()>> LVEvtDesc;

class LookWidget;
class LookViewWidget;

class LookViewTabWidget : public QTabWidget, public EventSource<LVEvtDesc>
{
  public:
    enum Evt { Select = 0, Reset };

  public:
    LookViewTabWidget(QWidget *parent = nullptr);

  public:
    LookViewWidget *currentView();
    void setLookWidget(LookWidget *lw);

    void showFolder(const QString &path);
    void updateViews();

  private:
    void selectionChanged();
    void tabChanged(int index);
    void tabClosed(int index);
    TupleT<bool, uint16_t> tabExists(const QString &name);

  private:
    LookWidget *m_lookWidget;
};