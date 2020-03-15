#include "shadow_widget.h"
#include "ui/interfaces/frameinterface.h"

#include <QPainter>
#include <QStackedLayout>
#include <QVBoxLayout>

namespace installer {

ShadowWidget::ShadowWidget(QWidget* parent)
    : QWidget(parent)
    , childFrameInterface(nullptr)
    , m_centerLayout(new QStackedLayout)
    , m_mainLayout(new QVBoxLayout)
{
    QWidget* widget = new QWidget;
    widget->setLayout(m_centerLayout);
    m_mainLayout->addWidget(widget, 0, Qt::AlignCenter);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_mainLayout);
}

void installer::ShadowWidget::setContent(BaseFrameInterface* inter)
{
    Q_ASSERT(inter != nullptr);

    if (childFrameInterface) {
        eraseContent();
    }

    childFrameInterface = inter;
    m_centerLayout->addWidget(childFrameInterface);
}

void ShadowWidget::eraseContent()
{
    if (childFrameInterface) {
        m_centerLayout->removeWidget(childFrameInterface);
        childFrameInterface->hide();
    }

    childFrameInterface = nullptr;
}

void installer::ShadowWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(0, 0, 0, 125));

    QWidget::paintEvent(event);
}

}
