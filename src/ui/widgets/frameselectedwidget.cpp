#include "frameselectedwidget.h"
#include "ui/interfaces/frameinterface.h"
#include "base/file_util.h"
#include "ui/utils/widget_util.h"

#include <QRadioButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>

namespace installer {
FrameSelectedWidget::FrameSelectedWidget(FrameInterface* frame, QWidget *parent)
    : QFrame(parent)
    , m_frame(frame)
    , m_state(FrameLabelState::Initial)
{
    m_noLabel = new QLabel;
    m_noLabel->setFixedSize(12, 12);
    m_noLabel->setPixmap(installer::renderPixmap(":/images/NO_inactive.svg"));

    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setFixedWidth(220);

    m_checkedLabel = new QLabel;
    m_checkedLabel->setFixedSize(12, 12);
    m_checkedLabel->setPixmap(installer::renderPixmap(":/images/done_inactive.svg"));

    m_hLayout = new QHBoxLayout;
    m_hLayout->setMargin(0);
    m_hLayout->setSpacing(2);

    m_hLayout->addSpacing(10);
    m_hLayout->addWidget(m_noLabel, 0, Qt::AlignHCenter);
    m_hLayout->addSpacing(10);
    m_hLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft | Qt::AlignHCenter);
    m_hLayout->addWidget(m_checkedLabel, 0, Qt::AlignRight | Qt::AlignHCenter);
    m_hLayout->addSpacing(15);
    setLayout(m_hLayout);

    setFixedSize(300, 36);
}

void FrameSelectedWidget::setText(const QString &text)
{
    m_titleLabel->setText(text);
}

void FrameSelectedWidget::setNormalStyle()
{
    setStyleSheet("#frameSelectedWidget {background-color: rgba(255, 255, 255, 0.1);}");
}

void FrameSelectedWidget::setShowStyle()
{
    setStyleSheet("background-color: blue;");
}

void FrameSelectedWidget::setBackable(bool backable)
{
//    m_radioBotton->setCheckable(backable);
//    m_radioBotton->setChecked(backable);
}

bool FrameSelectedWidget::isBackable() const
{
//    return m_radioBotton->isCheckable();
    return true;
}

void FrameSelectedWidget::mousePressEvent(QMouseEvent *event)
{
//    if (!m_radioBotton->isCheckable()){
//        return;
//    }

    emit frameClicked(m_frame);
}
}

