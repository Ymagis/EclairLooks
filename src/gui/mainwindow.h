#pragma once

#include <QtWidgets/QMainWindow>


class DevWidget;
class LookWidget;
class LogWidget;
class SettingWidget;

// Main gui class holding the different tabs of the application
class MainWindow : public QMainWindow
{
  public:
    MainWindow(QWidget *parent = nullptr);

  public:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    QSize sizeHint() const override;

  public:
    void setup();
    void centerOnScreen();

  private:
    void setupHelp();

  private:
    QMenu *m_fileMenu;

    QTabWidget *m_tabWidget;
    LogWidget *m_logWidget;
    DevWidget *m_devWidget;
    LookWidget *m_lookWidget;
    SettingWidget *m_settingWidget;
};