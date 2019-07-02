#include <locale>

#include <QtWidgets/QApplication>
#include <QtGui/QSurfaceFormat>
#include <QtWidgets/QDesktopWidget>
#include <QFile>

#include <context.h>
#include <gui/mainwindow.h>
#include <operator/ocio/matrix.h>
#include <operator/ocio/filetransform.h>
#include <operator/ocio/colorspace.h>
#include <operator/ctl/operator.h>


void setupContext()
{
    // Settings
    using FP = FilePathParameter;
    ParameterSerialList& s = Context::getInstance().settings();
    s.Add<FP>("Default CTL Folder", "", "Choose a folder", "", FP::PathType::Folder);
    s.Add<FP>("Default OCIO Config", "", "Choose an ocio config file", "");
    s.Add<FP>("Default Image", "", "Choose an image", "");
    s.Add<FP>("Image Base Folder", "", "Choose a folder", "", FP::PathType::Folder);
    s.Add<FP>("Look Base Folder", "", "Choose a folder", "", FP::PathType::Folder);
    s.Add<FP>("Look Tonemap LUT", "", "Choose a LUT", "");

    // Pipeline
    ImagePipeline& p = Context::getInstance().pipeline();
    p.SetName("main");

    QFile f = QFile(":/images/stresstest.exr");
    QByteArray blob;
    if (f.open(QIODevice::ReadOnly))
        blob = f.readAll();
    if (Image img = Image::FromBuffer((void *) blob.data(), blob.count()))
        p.SetInput(img);

    // Operators
    ImageOperatorList& o = Context::getInstance().operators();
    o.Register<OCIOMatrix>();
    o.Register<OCIOFileTransform>();
    o.Register<OCIOColorSpace>();
    o.Register<CTLTransform>();
}

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName("Eclair Looks");
    QCoreApplication::setOrganizationName("Ymagis");
    QCoreApplication::setOrganizationDomain("ymagis.com");
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QSurfaceFormat format;
    format.setSamples(16);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
    format.setSwapInterval(1);
    format.setVersion(3, 2);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    setupContext();

    std::string imgPath =
        Context::getInstance().settings()
        .Get<FilePathParameter>("Default Image")->value();
    if(Image img = Image::FromFile(imgPath))
        Context::getInstance().pipeline().SetInput(img);

    QApplication app(argc, argv);

    // Make sure we use C locale to parse float
    // This need to be placed right after QApplication initialization
    // See : http://doc.qt.io/qt-5/qcoreapplication.html#locale-settings
    QLocale::setDefault(QLocale("C"));
    setlocale(LC_NUMERIC, "C");

    QFile cssFile(":/css/application.css");
    cssFile.open(QFile::ReadOnly);
    QString cssString = QLatin1String(cssFile.readAll());
    app.setStyleSheet(cssString);

    MainWindow mainWindow;
    mainWindow.setup();
    mainWindow.show();
    mainWindow.centerOnScreen();

    Context::getInstance().pipeline().Init();

    return app.exec();
}
