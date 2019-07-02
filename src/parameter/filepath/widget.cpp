#include "widget.h"
#include "parameter.h"

#include <QtWidgets/QWidget>
#include <QtCore/QFileInfo>


ParameterFilePathWidget::ParameterFilePathWidget(Parameter *param, QWidget *parent)
    : ParameterWidget(param, parent)
{
    m_filePathParam = static_cast<FilePathParameter *>(param);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit();
    m_toolButton = new QToolButton();
    hLayout->addWidget(m_lineEdit);
    hLayout->addWidget(m_toolButton);
    m_layout->addLayout(hLayout);

    updateWidget(*param);

    QObject::connect(m_toolButton, &QToolButton::clicked,
                        [&, p = m_filePathParam, w = this, le = m_lineEdit]() {
                            QString path = QDir(QString::fromStdString(p->value())).path();
                            QString fileName;

                            if (p->pathType() == FilePathParameter::PathType::File)
                                fileName = QFileDialog::getOpenFileName(
                                    w, QString::fromStdString(p->description()), path,
                                    QString::fromStdString(p->filters()));
                            else
                                fileName = QFileDialog::getExistingDirectory(
                                    w, QString::fromStdString(p->description()), path);

                            if (!fileName.isEmpty()) {
                                p->setValue(fileName.toStdString());
                                le->setText(QString::fromStdString(p->value()));
                            }
                        });

    QObject::connect(m_lineEdit, &QLineEdit::editingFinished,
                        [&, p = m_filePathParam, le = m_lineEdit]() {
                            std::string currentText = le->text().toStdString();
                            if (currentText != p->value()) {
                                p->setValue(currentText);
                                UpdateLineEdit(*p);
                            }
                        });
}

void ParameterFilePathWidget::updateWidget(const Parameter &p)
{
    const FilePathParameter *fpp = static_cast<const FilePathParameter *>(&p);

    QString path = QString::fromStdString(fpp->value());
    m_lineEdit->setText(path);
    m_toolButton->setText("...");
    UpdateLineEdit(p);
}

void ParameterFilePathWidget::UpdateLineEdit(const Parameter &p)
{
    const FilePathParameter *fpp = static_cast<const FilePathParameter *>(&p);

    using PathType = FilePathParameter::PathType;
    QString path = QString::fromStdString(fpp->value());
    bool fileMissing = m_filePathParam->pathType() == PathType::File && !QFileInfo::exists(path);
    bool dirMissing  = m_filePathParam->pathType() == PathType::Folder && !QDir(path).exists();

    if (fileMissing || dirMissing)
        m_lineEdit->setStyleSheet(QString("color: red"));
    else
        m_lineEdit->setStyleSheet("");
}
