#include "widget.h"

#include <iostream>

#include <QtCore/QDateTime>


static LogWidget * thisInstance = nullptr;

void LogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!thisInstance)
        return;

    QString color;
    switch (type) {
    case QtDebugMsg:
      color = "blue";
      break;
    case QtInfoMsg:
      color = "green";
      break;
    case QtWarningMsg:
      color = "orange";
      break;
    case QtCriticalMsg:
      color = "red";
      break;
    case QtFatalMsg:
      color = "black";
    }

    QDateTime now = QDateTime::currentDateTime();
    QString msgHtml = msg;
    msgHtml.replace("\n", "<br>");
    QString textHtml = QString("<font color=\"%1\">%2 : %3</font>")
                       .arg(color, now.toString("hh:mm:ss.z"), msgHtml);
    thisInstance->appendHtml(textHtml);

    QString text = QString("Eclair Looks : %1 : %2").arg(now.toString("hh:mm:ss.z"), msg);
    std::cout << text.toStdString() << std::endl;
}

LogWidget::LogWidget(QWidget *parent)
: QPlainTextEdit(parent)
{
    thisInstance = this;
    qInstallMessageHandler(LogHandler);

    setReadOnly(true);
}

LogWidget::~LogWidget()
{
    qInstallMessageHandler(0);
}
