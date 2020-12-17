#include "partition_table_widget.h"
#include "ui/utils/widget_util.h"
#include <QStyleOption>
#include <QPainter>
#include <QEvent>
#include <QPainterPath>

namespace installer {

namespace {
    const int kPartitionTableWarningWidgetWidth = 470;
    const int kPartitionTableWarningWidgetMinHeight = 98;
    const int KQLabelWidth = 402;
    const int kTitleFont = 14; // 14pt
    const int kDescFont = 12; // 12pt
}

PartitionTableWarningWidget::PartitionTableWarningWidget(QWidget *parent)
    : DButtonBoxButton ("", parent)
    , m_isflag(false)
{
    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setMinimumWidth(KQLabelWidth);

    QFont titleFont;
    titleFont.setPixelSize(kTitleFont);
    m_titleLabel->setFont(titleFont);

    m_descLabel = new QLabel;
    m_descLabel->setObjectName("descLabel");
    m_descLabel->setMinimumWidth(KQLabelWidth);
    m_descLabel->setWordWrap(true);
    m_descLabel->adjustSize();

    QFont descFont;
    descFont.setPixelSize(kDescFont);
    m_descLabel->setFont(descFont);

    m_vLayout = new QVBoxLayout;
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setSpacing(0);
    m_vLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);
    m_vLayout->addWidget(m_descLabel, 0, Qt::AlignLeft);

    m_checkedLabel = new QLabel;
    const QPixmap pixmap = installer::renderPixmap(":/images/select.svg");
    Q_ASSERT(!pixmap.isNull());
    m_checkedLabel->setPixmap(pixmap);
    m_checkedLabel->setFixedSize(pixmap.size() / devicePixelRatioF());
    m_checkedLabel->setVisible(isChecked());

    m_hLayout = new QHBoxLayout;
    m_hLayout->setContentsMargins(10, 10, 10, 10);
    m_hLayout->setSpacing(0);

    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addStretch();
    m_hLayout->addWidget(m_checkedLabel, 0, Qt::AlignRight);

    setMinimumWidth(kPartitionTableWarningWidgetWidth);
    setMinimumHeight(kPartitionTableWarningWidgetMinHeight);
    setObjectName("PartitionTableWarningWidget");
    setContentsMargins(0, 0, 0, 0);
    setLayout(m_hLayout);
}

void PartitionTableWarningWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath PaintPath;
    QColor color(0, 0, 0, 15);

    if (m_isflag) {
        color = QColor(0, 0, 0, 55);
    }

    PaintPath.addRoundedRect(rect(), 10, 10);
    painter.fillPath(PaintPath, color);

    QWidget::paintEvent(event);
}

void PartitionTableWarningWidget::enterEvent(QEvent* event)
{
    m_isflag = true;

    update();

    QWidget::leaveEvent(event);
}

void PartitionTableWarningWidget::leaveEvent(QEvent* event)
{
    m_isflag = false;

    update();

    QWidget::leaveEvent(event);
}

void PartitionTableWarningWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void PartitionTableWarningWidget::setDesc(const QString &desc)
{
    m_descLabel->setText(desc);
}

void PartitionTableWarningWidget::updateCheckedAppearance()
{
    m_checkedLabel->setVisible(isChecked());
}

}
