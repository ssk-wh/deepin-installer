#include "component_widget.h"
#include "base/file_util.h"
#include <QStyleOption>
#include <QPainter>
#include <QEvent>
#include <QPainterPath>

namespace installer {

namespace {
    const int kComponentWidgetWidth = 250;
    const int kComponentWidgetMinHeight = 70;
    const int KQLabelWidth = 200;
}

ComponentWidget::ComponentWidget(bool singleSelected, QWidget *parent)
    : QFrame (parent)
    , m_radioBotton(nullptr)
    , m_checkBox(nullptr)
    , m_isHead(false)
    , m_isTail(false)
    , m_isflag(false)
{
    QFont font;
    font.setWeight(QFont::DemiBold);

    m_titleLabel = new QLabel;
    m_titleLabel->setFixedWidth(KQLabelWidth);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->adjustSize();
    m_titleLabel->setFont(font);

    m_descLabel = new QLabel;
    m_descLabel->setFixedWidth(KQLabelWidth);
    m_descLabel->setWordWrap(true);
    m_descLabel->adjustSize();

    m_hLayout = new QHBoxLayout;
    m_hLayout->addSpacing(0);
    m_vLayout = new QVBoxLayout;
    m_vLayout->setSpacing(0);
    m_vLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);
    m_vLayout->addWidget(m_descLabel, 0, Qt::AlignLeft);
    if(singleSelected){
        m_radioBotton = new QRadioButton;
        m_radioBotton->setCheckable(true);
        m_radioBotton->setChecked(false);
        m_radioBotton->installEventFilter(this);
        m_radioBotton->setFocusPolicy(Qt::TabFocus);
        m_hLayout->addWidget(m_radioBotton, 0, Qt::AlignLeft | Qt::AlignVCenter);

        connect(m_radioBotton, &QRadioButton::clicked, this
                , &ComponentWidget::clicked);
    }
    else {
        m_checkBox = new QCheckBox;
        m_checkBox->setCheckable(true);
        m_checkBox->setChecked(false);
        m_checkBox->installEventFilter(this);
        m_checkBox->setFocusPolicy(Qt::TabFocus);
        m_hLayout->addWidget(m_checkBox, 0, Qt::AlignLeft | Qt::AlignVCenter);

        connect(m_checkBox, &QCheckBox::clicked, this
                , &ComponentWidget::clicked);
    }
    m_hLayout->addSpacing(0);
    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addStretch();

    setMinimumWidth(kComponentWidgetWidth);
    setMinimumHeight(kComponentWidgetMinHeight);
    setObjectName("ComponentWidget");
    setLayout(m_hLayout);

    this->setStyleSheet(ReadFile(":/styles/component_widget.css"));
}

void ComponentWidget::setSelected(bool selected)
{
    if (m_radioBotton) {
        m_radioBotton->setChecked(selected);
    }
    else {
        m_checkBox->setChecked(selected);
    }
}

void ComponentWidget::enterEvent(QEvent* event)
{
    m_isflag = true;

    update();

    QFrame::leaveEvent(event);
}

void ComponentWidget::leaveEvent(QEvent* event)
{
    m_isflag = false;

    update();

    QFrame::leaveEvent(event);
}

bool ComponentWidget::isSelected() const
{
    return m_radioBotton ? m_radioBotton->isChecked() : m_checkBox->isChecked();
}

void ComponentWidget::setIsHead(bool head)
{
    m_isHead = head;
}

void ComponentWidget::setIsTail(bool tail)
{
    m_isTail = tail;
}

void ComponentWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void ComponentWidget::setDesc(const QString &desc)
{
   m_descLabel->setText(desc);
}

void ComponentWidget::mousePressEvent(QMouseEvent *event)
{
    setSelected(!isSelected());

    QFrame::mousePressEvent(event);

    emit clicked();
}

bool ComponentWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if(event->type() == QEvent::KeyPress){
        return true;
    }

    return QWidget::eventFilter(watched, event);
}

}
