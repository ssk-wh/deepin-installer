#include "partition_color_label.h"

#include <QPainter>

PartitionColorLabel::PartitionColorLabel(QColor color, QWidget *parent)
    : QWidget(parent)
    , m_color(color)
{
    setFixedSize(10, 10);
}

void PartitionColorLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.fillRect(rect(), m_color);

    QWidget::paintEvent(event);
}
