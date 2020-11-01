#include "shadow_widget.h"
#include "ui/interfaces/frameinterface.h"

#include <QPainter>
#include <QStackedLayout>
#include <QVBoxLayout>

namespace installer {

ShadowWidget::ShadowWidget(QWidget* parent)
    : QWidget(parent)
    , m_childFrameInterface(nullptr)
    , m_centerLayout(new QStackedLayout)
{
    m_centerLayout->setContentsMargins(0, 0, 0, 0);
    m_centerLayout->setSpacing(0);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(m_centerLayout);

    m_widget = new QWidget;
    m_widget->setContentsMargins(0, 0, 0, 0);
    m_widget->setLayout(layout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addStretch();
    mainLayout->addWidget(m_widget, 0, Qt::AlignHCenter);
    mainLayout->addStretch();

    setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

void installer::ShadowWidget::setContent(BaseFrameInterface* inter)
{
    Q_ASSERT(inter != nullptr);

    if (m_childFrameInterface) {
        eraseContent();
    }

    m_childFrameInterface = inter;
    m_widget->setFixedSize(m_childFrameInterface->size());
    m_centerLayout->addWidget(m_childFrameInterface);
}

void ShadowWidget::eraseContent()
{
    if (m_childFrameInterface) {
        m_centerLayout->removeWidget(m_childFrameInterface);
        m_childFrameInterface->hide();
    }

    m_childFrameInterface = nullptr;
}

void installer::ShadowWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(0, 0, 0, 125));

    QWidget::paintEvent(event);
}

}
