#include "shadow_widget.h"
#include "ui/interfaces/frameinterface.h"

#include <QPainter>
#include <QStackedLayout>
#include <QVBoxLayout>

namespace installer {

ShadowWidget::ShadowWidget(QWidget* parent)
    : QWidget(parent)
    , m_childFrameInterface(nullptr)   
{
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    //this->setAttribute(Qt::WA_TranslucentBackground);
    m_centerLayout = new QVBoxLayout;
    m_centerLayout->setContentsMargins(0, 0, 0, 0);
    m_centerWidget = new QWidget(this);
    m_centerWidget->setObjectName("centerWidget");
    m_centerWidget->setStyleSheet("QWidget#centerWidget{ background-color:rgba(0, 0, 0, 125); }");
    m_centerWidget->setLayout(m_centerLayout);
    QVBoxLayout *m_Layout = new QVBoxLayout;
    m_Layout->setContentsMargins(0, 0, 0, 0);
    m_Layout->addWidget(m_centerWidget);
    this->setLayout(m_Layout);

}

void installer::ShadowWidget::setContent(BaseFrameInterface* inter)
{
    Q_ASSERT(inter != nullptr);

    if (m_childFrameInterface) {
        eraseContent();
    }

    m_childFrameInterface = inter;
    m_centerLayout->addWidget(m_childFrameInterface, 0, Qt::AlignCenter);
}

void ShadowWidget::eraseContent()
{
    if (m_childFrameInterface) {
        m_centerLayout->removeWidget(m_childFrameInterface);
    }

    m_childFrameInterface = nullptr;
}

}
