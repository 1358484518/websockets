#ifndef OCPPCTRL_H
#define OCPPCTRL_H

#include <QWidget>
#include "QLabel"
#include "QHBoxLayout"
#include "QLineEdit"
#include "QPushButton"
#include "QList"

class LabeledEdit : public QWidget
{
    Q_OBJECT
public:
    explicit LabeledEdit(const QString &labelText, const QString &btnText, QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QHBoxLayout *lay = new QHBoxLayout(this);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(6);

        m_label = new QLabel(labelText);
        m_label->setAlignment(Qt::AlignLeft );
        m_label->setMinimumWidth(220);
        m_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        m_edit = new QLineEdit;

        m_btn = new QPushButton(btnText);
        m_btn->setFixedWidth(80);

        lay->addWidget(m_label);
        lay->addWidget(m_edit, 1);
        lay->addWidget(m_btn);

        connect(m_btn, &QPushButton::clicked, this, &LabeledEdit::buttonClicked);
        connect(m_edit, &QLineEdit::textChanged, this, &LabeledEdit::textChanged);
        connect(m_edit, &QLineEdit::editingFinished, this, &LabeledEdit::editingFinished);
    }

    // ===== 文本操作 =====
    QString text() const                    { return m_edit->text(); }
    void setText(const QString &t)          { m_edit->setText(t); }
    void clear()                            { m_edit->clear(); }

    // ===== 显示文本 =====
    void setLabelText(const QString &t)     { m_label->setText(t); }
    void setButtonText(const QString &t)    { m_btn->setText(t); }
    void setPlaceholderText(const QString &t){ m_edit->setPlaceholderText(t); }

    // ===== 状态控制 =====
    void setReadOnly(bool r)                { m_edit->setReadOnly(r); }
    void setButtonEnabled(bool e)           { m_btn->setEnabled(e); }
    void setEditEnabled(bool e)             { m_edit->setEnabled(e); }
    void setEnabled(bool e)         { m_edit->setEnabled(e); m_btn->setEnabled(e); QWidget::setEnabled(e); }

    // ===== 输入限制 =====
    void setMaxLength(int len)              { m_edit->setMaxLength(len); }
    void setValidator(const QValidator *v)  { m_edit->setValidator(v); }
    void setEchoMode(QLineEdit::EchoMode m) { m_edit->setEchoMode(m); }

    // ===== 标签宽度（统一对齐用）=====
    int labelWidth() const                  { return m_label->sizeHint().width(); }
    void setLabelWidth(int w)               { m_label->setFixedWidth(w); }

signals:
    void buttonClicked();
    void textChanged(const QString &text);
    void editingFinished();

private:
    QLabel      *m_label;
    QLineEdit   *m_edit;
    QPushButton *m_btn;
};
class OcppCtrlWidget : public QWidget
{
    Q_OBJECT
public:
    struct OcppActionInfo{
      QString name;
      QString info;
      QString action;
    };
public:
    explicit OcppCtrlWidget(QWidget *parent = nullptr);

signals:
private:
    QList<LabeledEdit*> listLabeledEdit;
};

#endif // OCPPCTRL_H
