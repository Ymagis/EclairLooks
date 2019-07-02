#pragma once

#include <QtWidgets/QWidget>

#include <utils/event_source.h>


class LookWidget;
class LookViewWidget;
class QToolButton;

typedef EventDesc <
    FuncT<void(const QString &)>,
    FuncT<void()>> LVEvtDesc;

// TODO : should be a common widget because will be shared by image / looks
class LookSelectionWidget : public QWidget, public EventSource<LVEvtDesc>
{
  public:
    enum Evt { Select = 0, Reset };

  public:
    LookSelectionWidget(QWidget *parent = nullptr);

  public:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  public:
    void setLookWidget(LookWidget *lw);

    LookViewWidget * viewWidget();

    void updateSelection();
    void clearSelection();
    void saveSelection();
    void loadSelection();

  private:
    LookViewWidget *m_viewWidget;

    QToolButton *m_saveBtn;
    QToolButton *m_clearBtn;
    QToolButton *m_loadBtn;
};