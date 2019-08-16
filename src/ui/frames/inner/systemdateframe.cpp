/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <justforlxz@outlook.com>
 *
 * Maintainer: justforlxz <justforlxz@outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "systemdateframe.h"

#include "ui/widgets/nav_button.h"
#include "ui/widgets/pointer_button.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QEvent>

namespace installer {
class SystemDateFramePrivate : public QObject{
    Q_OBJECT
public:
    SystemDateFramePrivate(SystemDateFrame* qq) : ptr(qq) {}

    QLabel* m_title = new QLabel;
    QLabel* m_subTitle = new QLabel;
    QCheckBox* m_autoSyncTimeCheckBox = new QCheckBox;
    NavButton* m_cancelBtn = new NavButton;
    NavButton* m_acceptBtn = new NavButton;
    QLineEdit* m_hourEdit = new QLineEdit;
    QLineEdit* m_minuteEdit = new QLineEdit;
    QLineEdit* m_yearEdit = new QLineEdit;
    QLineEdit* m_monthEdit = new QLineEdit;
    QLineEdit* m_dayEdit = new QLineEdit;

    QLabel* m_hourLabel = new QLabel;
    QLabel* m_minuteLabel = new QLabel;
    QLabel* m_yearLabel = new QLabel;
    QLabel* m_monthLabel = new QLabel;
    QLabel* m_dayLabel = new QLabel;

    SystemDateFrame* ptr;

    void init();

    void updateTs() {
        m_hourLabel->setText(tr("Hour"));
        m_minuteLabel->setText(tr("Minute"));
        m_yearLabel->setText(tr("Year"));
        m_monthLabel->setText(tr("Month"));
        m_dayLabel->setText(tr("Day"));
    }
};

SystemDateFrame::SystemDateFrame(QWidget *parent)
    : QWidget(parent)
    , d_private(new SystemDateFramePrivate(this))
{
    d_private->init();
    d_private->updateTs();
}

SystemDateFrame::~SystemDateFrame()
{

}

bool SystemDateFrame::event(QEvent *event)
{
    Q_D(SystemDateFrame);

    if (event->type() == QEvent::LanguageChange) {
        d->updateTs();
    }

    return QWidget::event(event);
}

void SystemDateFrame::readConf()
{

}

void SystemDateFrame::writeConf()
{

}

void SystemDateFramePrivate::init()
{
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    QVBoxLayout* centerLayout = new QVBoxLayout;
    centerLayout->setMargin(0);
    centerLayout->setSpacing(0);

    centerLayout->addWidget(m_title, 0, Qt::AlignHCenter);
    centerLayout->addWidget(m_subTitle, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(30);
    centerLayout->addWidget(m_autoSyncTimeCheckBox, 0, Qt::AlignHCenter);

    m_autoSyncTimeCheckBox->setCheckState(Qt::Checked);
    m_autoSyncTimeCheckBox->setCheckable(true);
    m_autoSyncTimeCheckBox->setChecked(true);

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
    ptr->setLayout(mainLayout);

    auto createWidgetWithBg = [=](QWidget* content) -> QWidget* {
        QWidget* bgWidget = new QWidget;
        QHBoxLayout* layout = new QHBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget(content);
        bgWidget->setLayout(layout);
        bgWidget->setObjectName("bgWidget");
        return bgWidget;
    };

    QWidget* hourWidget = createWidgetWithBg(m_hourEdit);
    QWidget* minuteWidget = createWidgetWithBg(m_minuteEdit);

    QHBoxLayout* timeLayout = new QHBoxLayout;
    timeLayout->setSpacing(0);
    timeLayout->setMargin(0);
    timeLayout->addWidget(hourWidget);
    timeLayout->addWidget(m_hourLabel);
    timeLayout->addSpacing(30);
    timeLayout->addWidget(minuteWidget);
    timeLayout->addWidget(m_minuteLabel);

    centerLayout->addLayout(timeLayout);

    QHBoxLayout* yearLayout = new QHBoxLayout;
    yearLayout->setSpacing(0);
    yearLayout->setMargin(0);

    PointerButton* minusYearBtn = new PointerButton;
    PointerButton* addYearBtn   = new PointerButton;

    yearLayout->addWidget(minusYearBtn);
    yearLayout->addWidget(m_yearEdit);
    yearLayout->addWidget(m_yearLabel);
    yearLayout->addWidget(addYearBtn);

    QWidget* yearWidget = new QWidget;
    yearWidget->setObjectName("bgWidget");
    yearWidget->setLayout(yearLayout);

    QHBoxLayout* monthLayout = new QHBoxLayout;
    monthLayout->setSpacing(0);
    monthLayout->setMargin(0);

    PointerButton* minusMouthBtn = new PointerButton;
    PointerButton* addMouthBtn = new PointerButton;

    monthLayout->addWidget(minusMouthBtn);
    monthLayout->addWidget(m_monthEdit);
    monthLayout->addWidget(m_monthLabel);
    monthLayout->addWidget(addMouthBtn);

    QWidget* monthWidget = new QWidget;
    monthWidget->setObjectName("bgWidget");
    monthWidget->setLayout(monthLayout);

    QHBoxLayout* dayLayout = new QHBoxLayout;
    dayLayout->setSpacing(0);
    dayLayout->setMargin(0);
    PointerButton* minusDayBtn = new PointerButton;
    PointerButton* addDayBtn = new PointerButton;
    dayLayout->addWidget(minusDayBtn);
    dayLayout->addWidget(m_dayEdit);
    dayLayout->addWidget(m_dayLabel);
    dayLayout->addWidget(addDayBtn);

    QWidget* dayWidget = new QWidget;
    dayWidget->setObjectName("bgWidget");
    dayWidget->setLayout(dayLayout);

    centerLayout->addWidget(yearWidget);
    centerLayout->addWidget(monthWidget);
    centerLayout->addWidget(dayWidget);

    centerLayout->addStretch();

    centerLayout->addWidget(m_acceptBtn, 0, Qt::AlignHCenter);
    centerLayout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);

    connect(m_autoSyncTimeCheckBox, &QCheckBox::clicked, this, [=] {
        m_autoSyncTimeCheckBox->blockSignals(true);
        m_autoSyncTimeCheckBox->setCheckState(Qt::Checked);
        m_autoSyncTimeCheckBox->setChecked(true);
        m_autoSyncTimeCheckBox->blockSignals(false);

        emit ptr->cancel();
    });

    connect(m_cancelBtn, &NavButton::clicked, ptr, &SystemDateFrame::cancel);
    connect(m_acceptBtn, &NavButton::clicked, ptr, &SystemDateFrame::finished);
}
}

#include "systemdateframe.moc"
