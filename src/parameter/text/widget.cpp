#include "widget.h"
#include "parameter.h"


ParameterTextWidget::ParameterTextWidget(Parameter *param, QWidget *parent)
    : ParameterWidget(param, parent)
{
    m_textParam = static_cast<TextParameter *>(param);

    m_textEdit = new QTextEdit();
    m_layout->addWidget(m_textEdit);

    updateWidget(*param);

    QObject::connect(m_textEdit, &QTextEdit::textChanged,
                     [&, p = m_textParam, te = m_textEdit]() {
                         p->setValue(te->toPlainText().toStdString());
                     });
}

void ParameterTextWidget::updateWidget(const Parameter &p)
{
    const TextParameter *tp = static_cast<const TextParameter *>(&p);
    m_textEdit->setText(QString::fromStdString(tp->defaultValue()));
}
