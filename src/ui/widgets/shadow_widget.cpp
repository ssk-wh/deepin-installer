#include "shadow_widget.h"
#include "ui/interfaces/frameinterface.h"

#include <QPainter>
#include <QVBoxLayout>

const int ChildFrameWidght = 698;
const int ChildFrameHeight = 678;

namespace installer {

ShadowWidget::ShadowWidget(QWidget* parent)
    : QWidget(parent)
    , childFrameInterface(nullptr)
    , m_centerLayout(new QVBoxLayout)
{
    m_centerLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_centerLayout);
}


void installer::ShadowWidget::setContent(ChildFrameInterface* inter)
{
    if (childFrameInterface) {
        m_centerLayout->removeWidget(childFrameInterface);
        childFrameInterface->hide();
    }

    childFrameInterface = inter;

    childFrameInterface->resize(ChildFrameWidght, ChildFrameHeight);
    m_centerLayout->addWidget(childFrameInterface, 0, Qt::AlignCenter);
}

void installer::ShadowWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(0, 0, 0, 125));

    QWidget::paintEvent(event);
}
}
