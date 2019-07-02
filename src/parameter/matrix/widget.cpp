#include "widget.h"


ParameterMatrixWidget::ParameterMatrixWidget(Parameter *param, QWidget *parent)
: ParameterWidget(param, parent)
{
    m_matrixParam = static_cast<MatrixParameter *>(param);

    m_gridLayout = new QGridLayout();

    m_validator = new QDoubleValidator(this);
    m_validator->setDecimals(5);
    m_validator->setNotation(QDoubleValidator::StandardNotation);

    Matrix4x4 defaults = m_matrixParam->defaultValue();
    for (int i = 0; i < defaults.size(); ++i) {
        QLineEdit *lineEdit = new QLineEdit();
        lineEdit->setValidator(m_validator);
        lineEdit->setText(QString::number(defaults[i]));
        m_gridLayout->addWidget(lineEdit, i / 4, i % 4);

        QObject::connect(lineEdit,QOverload<>::of(&QLineEdit::returnPressed),
            [&, p = m_matrixParam]() {
                p->setValue(getValues());
            });
    }

    QPushButton *clipboardBtn = new QPushButton(QIcon(QPixmap(":/icons/clipboard.png")), "");
    clipboardBtn->setToolTip("Past from clipboard");
    m_gridLayout->addWidget(clipboardBtn, 0, 5);
    QObject::connect(clipboardBtn, &QPushButton::pressed,
        [&, p = m_matrixParam]() {
            QClipboard *clipboard = QGuiApplication::clipboard();
            std::vector<double> vals = extractDoubles(clipboard->text());

            Matrix4x4 values = p->defaultValue();

            switch (vals.size()) {
                case 9:
                {
                    for (int i = 0; i < vals.size(); ++i)
                        values[(i / 3) * 4 + i % 3] = vals[i];
                }
                break;
                case 16:
                {
                    for (int i = 0; i < vals.size(); ++i)
                        values[(i / 4) * 4 + i % 4] = vals[i];
                }
                break;
                default:
                {
                    for (int i = 0; i < vals.size(); ++i) {
                        if (i >= 16) break;
                        values[(i / 4) * 4 + i % 4] = vals[i];
                    }
                }
            }

            setValues(values);
            p->setValue(values);
        });

        QPushButton *resetBtn = new QPushButton(QIcon(QPixmap(":/icons/reset.png")), "");
        resetBtn->setToolTip("Set identity matrix");
        m_gridLayout->addWidget(resetBtn, 1, 5);
        QObject::connect(resetBtn, &QPushButton::pressed,
            [&, p = m_matrixParam]() {
                p->setValue(p->defaultValue());
        });

        QPushButton *transposeBtn = new QPushButton(QIcon(QPixmap(":/icons/transpose.png")), "");
        transposeBtn->setToolTip("Transpose matrix");
        m_gridLayout->addWidget(transposeBtn, 2, 5);
        QObject::connect(transposeBtn, &QPushButton::pressed,
            [&, p = m_matrixParam]() {
                Matrix4x4 m = p->value();
                Matrix4x4 newVals;
                for (int col = 0; col < 4; ++col)
                    for (int row = 0; row < 4; ++row) {
                        newVals[col * 4 + row] = m[row * 4 + col];
                    }
                p->setValue(newVals);
        });

    m_layout->addLayout(m_gridLayout);

    updateWidget(*param);
}

void ParameterMatrixWidget::updateWidget(const Parameter &p)
{
    const MatrixParameter *mp = static_cast<const MatrixParameter *>(&p);
    setValues(mp->value());
}

Matrix4x4 ParameterMatrixWidget::getValues()
{
    Matrix4x4 values;

    for (int i = 0; i < 16; ++i) {
        QLayoutItem * layoutItem = m_gridLayout->itemAt(i);
        QLineEdit * lineEdit = static_cast<QLineEdit *>(layoutItem->widget());
        values[i] = lineEdit->text().toDouble();
    }

    return values;
}

void ParameterMatrixWidget::setValues(const Matrix4x4 &m)
{
    for (int i = 0; i < 16; ++i) {
        QLayoutItem * layoutItem = m_gridLayout->itemAt(i);
        QLineEdit * lineEdit = static_cast<QLineEdit *>(layoutItem->widget());
        lineEdit->setText(QString::number(m[i]));
    }
}

std::vector<double> ParameterMatrixWidget::extractDoubles(const QString &str)
{
    std::vector<double> res;

    QRegExp regExp("[+-]?\\d*\[\\.,]?\\d+");
    int index = 0;
    bool ok;

    while (index < str.size()) {
        int n = str.indexOf(regExp, index);
        if (n < 0) break;

        double val = regExp.cap(0).toDouble(&ok);
        if (!ok) continue;

        res.push_back(val);
        index = n + regExp.matchedLength();
    }

    return res;
}