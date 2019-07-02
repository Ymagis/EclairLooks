#include "mainwindow.h"

#include <QtWidgets/QtWidgets>

#include <version.h>
#include <context.h>
#include <gui/common/setting.h>
#include <gui/view/dev/widget.h>
#include <gui/view/look/widget.h>
#include <gui/view/log/widget.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(ELOOK_VERSION_PRETTY);

    QSettings settings;
    restoreGeometry(settings.value("mw/geometry").toByteArray());
    restoreState(settings.value("mw/windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (QMessageBox::Yes == QMessageBox::question(
        this, "Close Confirmation", "Are you sure to quit ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)) {
        QSettings settings;
        settings.setValue("mw/geometry", saveGeometry());
        settings.setValue("mw/windowState", saveState());
        QMainWindow::closeEvent(event);

        event->accept();
        return;
    }

    event->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
      case Qt::Key_Escape:
        close();
        break;
      default:
        QMainWindow::keyPressEvent(event);
  }
}

QSize MainWindow::sizeHint() const
{
    return QSize(1280, 800);
}

void MainWindow::setup()
{
    //
    // Setup
    //

    m_logWidget = new LogWidget();
    m_devWidget = new DevWidget();
    m_lookWidget = new LookWidget(this);
    m_settingWidget = new SettingWidget(
        &Context::getInstance().settings(), "General");

    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(m_lookWidget, "Look");
    m_tabWidget->addTab(m_devWidget, "Dev");
    m_tabWidget->addTab(m_settingWidget, "Setting");
    m_tabWidget->addTab(m_logWidget, "Log");
    setCentralWidget(m_tabWidget);

    setupHelp();

    //
    // Actions
    //

    QAction *exportAction = new QAction(QIcon(QPixmap(":/icons/hexa.png")), tr("Export"));

    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(exportAction);

    //
    // Connections
    //

    QObject::connect(
        exportAction, &QAction::triggered,
        [this]() {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save 3DLUT"), "", tr("Cube Files (*.cube)"));
            Context::getInstance().pipeline().ExportLUT(fileName.toStdString(), 64);
        }
    );
}

void MainWindow::centerOnScreen()
{
    // Move window to the center of the screen
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::setupHelp()
{
    QString html;
    QFile f = QFile(":/html/help.html");
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        html = in.readAll();
    }

    QTextEdit * helpWidget = new QTextEdit();
    helpWidget->setHtml(html);
    helpWidget->setReadOnly(true);

    m_tabWidget->addTab(helpWidget, "Help");
}