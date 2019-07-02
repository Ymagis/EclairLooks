#include "operator.h"

#include <vector>
#include <filesystem>
#include <system_error>

#include <QtWidgets/QtWidgets>
#include <QtCore/QDebug>

#include <context.h>
#include <core/image.h>
#include <core/imagepipeline.h>
#include "transform.h"

namespace fs = std::filesystem;

// TODO :
// 1 - Find a way to detect all CTL arguments
// 2 - Dynamically add corresponding parameters for the operator
//     GUI needs to reload the OperatorWidget... Is that possible ?
//     New event type for this use case?
// 3 - Why don't we use IlmImfCtl ? Might add some speeds because of multithreading


CTLTransform::CTLTransform()
{
    AddParameterByCategory<FilePathParameter>("CTL", "CTL Base", "", "Choose a folder", "", FilePathParameter::PathType::Folder);
    AddParameterByCategory<FilePathParameter>("CTL", "CTL File");
}

ImageOperator *CTLTransform::OpCreate() const
{
    CTLTransform * op = new CTLTransform();
    op->SetBase(Context::getInstance().settings().Get<FilePathParameter>("Default CTL Folder")->value());
    return op;
}

std::string CTLTransform::OpName() const
{
    return "CTL Transform";
}

std::string CTLTransform::OpLabel() const
{
    QFileInfo fileInfo(QString::fromStdString(m_ctlFile));
    if (!fileInfo.absoluteFilePath().isEmpty())
        return "CTL - " + fileInfo.fileName().toStdString();
    else
        return "CTL";
}

ImageOperator *CTLTransform::OpCreateFromPath(const std::string &filepath) const
{
    QFileInfo file = QFileInfo(QString::fromStdString(filepath));
    if (file.isFile() && file.suffix() == "ctl") {
        CTLTransform * op = new CTLTransform();
        op->SetBase(Context::getInstance().settings().Get<FilePathParameter>("Default CTL Folder")->value());
        op->GetParameter<FilePathParameter>("CTL File")->setValue(filepath);
        return op;
    }

    return nullptr;
}

void CTLTransform::OpApply(Image &img)
{
    CTLOperations ops;
    ops.push_back({m_ctlFile});

    CTLParameters global;
    ctl_parameter_t alpha_param;
    alpha_param.name = "aIn";
    alpha_param.count = 1;
    alpha_param.value[0] = 1.0f;
    global.push_back(alpha_param);

    try {
        transform(img, ops, global, m_searchsPath);
    }
    catch (Iex::ArgExc & e) {
        qWarning() << e.what();
    }
    catch (Iex::LogicExc & e) {
        qWarning() << e.what();
    }
    catch(...) {
        qWarning() << "Unknown exception";
    }
}

bool CTLTransform::OpIsIdentity() const
{
    return GetParameter<FilePathParameter>("CTL File")->value().empty();
}

void CTLTransform::OpUpdateParamCallback(const Parameter & op)
{
    if (op.name() == "CTL Base") {
        auto p = static_cast<const FilePathParameter *>(&op);
        m_searchsPath.clear();

        fs::path folder(p->value());
        std::error_code ec;
        fs::directory_iterator it(folder, ec);
        if (ec) {
            qInfo() << "CTL search path invalid : " << QString::fromStdString(p->value());
            qInfo() << QString::fromStdString(ec.message());
        }

        while (it != fs::directory_iterator{}) {
            fs::path pathItem = (*it).path();
            if (fs::is_directory(pathItem)) {
                qInfo() << QString::fromStdString(pathItem.string());
                m_searchsPath.push_back(pathItem.string());
            }
            ++it;
        }
    }
    else if (op.name() == "CTL File") {
        auto p = static_cast<const FilePathParameter *>(&op);
        m_ctlFile = p->value();
    }
}

void CTLTransform::SetBase(const std::string &folder)
{
    GetParameter<FilePathParameter>("CTL Base")->setValue(folder);
}
