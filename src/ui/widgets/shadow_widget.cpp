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
    m_centerLayout->setContentsMargins(0, 0, 0, 0);
    m_centerLayout->setSpacing(0);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(m_centerLayout);

    widget = new QWidget;
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(layout);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_mainLayout->addStretch();
    m_mainLayout->addWidget(widget, 0, Qt::AlignHCenter);
    m_mainLayout->addStretch();

    setContentsMargins(0, 0, 0, 0);
    setLayout(m_mainLayout);
}

void installer::ShadowWidget::setContent(BaseFrameInterface* inter)
{
    Q_ASSERT(inter != nullptr);

    if (childFrameInterface) {
        eraseContent();
    }

    childFrameInterface = inter;
    widget->setFixedSize(childFrameInterface->size());
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
