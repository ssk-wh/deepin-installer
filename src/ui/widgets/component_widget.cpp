#include "component_widget.h"
#include "base/file_util.h"
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

namespace installer {

namespace {
    const int kComponentWidgetWidth = 468;
    const int kComponentWidgetMinHeight = 80;
    const int KQLabelWidth =260;

    const int kComponentWidgetReduceX = 9;
    const int kComponentWidgetReduceY = 7;
    const int kComponentWidgetReduceWH = 20;
}

ComponentWidget::ComponentWidget(bool singleSelected, QWidget *parent)
    : QFrame (parent)
    , m_radioBotton(nullptr)
    , m_checkBox(nullptr)
    , m_isHead(false)
    , m_isTail(false)
    , m_isflag(false)
{
    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setFixedWidth(KQLabelWidth);
    m_descLabel = new QLabel;
    m_descLabel->setObjectName("descLabel");
    m_descLabel->setFixedWidth(KQLabelWidth);
    m_descLabel->setWordWrap(true);
    m_hLayout = new QHBoxLayout;
    m_hLayout->addSpacing(15);
    m_vLayout = new QVBoxLayout;
    m_vLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);
    m_vLayout->addWidget(m_descLabel, 0, Qt::AlignLeft);
    if(singleSelected){
        m_radioBotton = new QRadioButton;
        m_radioBotton->setObjectName("radioBotton");
        m_radioBotton->setCheckable(true);
        m_radioBotton->setChecked(false);
        m_radioBotton->installEventFilter(this);
        m_radioBotton->setFocusPolicy(Qt::NoFocus);
        m_hLayout->addWidget(m_radioBotton, 0, Qt::AlignLeft | Qt::AlignVCenter);

        connect(m_radioBotton, &QRadioButton::clicked, this
                , &ComponentWidget::clicked);
    }
    else {
        m_checkBox = new QCheckBox;
        m_checkBox->setObjectName("checkBox");
        m_checkBox->setCheckable(true);
        m_checkBox->setChecked(false);
        m_checkBox->installEventFilter(this);
        m_checkBox->setFocusPolicy(Qt::NoFocus);
        m_hLayout->addWidget(m_checkBox, 0, Qt::AlignLeft | Qt::AlignVCenter);

        connect(m_checkBox, &QCheckBox::clicked, this
                , &ComponentWidget::clicked);
    }
    m_hLayout->addSpacing(5);
    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addStretch();

    setLayout(m_hLayout);
    setMaximumWidth(kComponentWidgetWidth);
    setMinimumHeight(kComponentWidgetMinHeight);
    setObjectName("ComponentWidget");
    setLayout(m_hLayout);
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

void ComponentWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath PaintPath;
    QColor color(0, 0, 0, 15);

    if (m_isflag) {
        color = QColor(0, 0, 0, 55);
    }

    PaintPath.addRoundedRect(rect().x() + kComponentWidgetReduceX, rect().y() + kComponentWidgetReduceY,
                             rect().width() - kComponentWidgetReduceWH, rect().height() - kComponentWidgetReduceWH,
                             11, 11);
    painter.fillPath(PaintPath, color);

    QWidget::paintEvent(event);
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
