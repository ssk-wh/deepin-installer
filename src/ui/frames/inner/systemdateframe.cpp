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
#include "base/file_util.h"

#include "ui/widgets/nav_button.h"
#include "ui/widgets/pointer_button.h"
#include "ui/widgets/line_edit.h"
#include "ui/widgets/title_label.h"
#include "service/settings_manager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QEvent>
#include <QChar>
#include <QProcess>
#include <QRegExpValidator>
#include <QDebug>
#include <functional>

namespace installer {

namespace {
    int kHourMinuteQLineEditWidth = 120;
    int kHourMinuteQLabelWidth = 40;
    int kYearMonthDayQPushButtonWidth = 35;
    int kYearMonthDayQLineEditWidth = 160;
    int kYearMonthDayQLabelWidth = 110;
    int kYearMonthDayQWidgetWidth = 340;
}

class SystemDateFramePrivate : public QObject{
    Q_OBJECT
public:
    SystemDateFramePrivate(SystemDateFrame* qq) : m_ptr(qq) {}

    QLabel* m_title = new TitleLabel(tr(""));
    QLabel* m_subTitle = new QLabel();
    QCheckBox* m_autoSyncTimeCheckBox = new QCheckBox;

    QLineEdit* m_hourEdit = new QLineEdit;
    QLineEdit* m_minuteEdit = new QLineEdit;
    PointerButton* m_addYearBtn   = new PointerButton;
    PointerButton* m_minusYearBtn = new PointerButton;
    QLineEdit* m_yearEdit = new QLineEdit;
    PointerButton* m_addMonthBtn = new PointerButton;
    PointerButton* m_minusMonthBtn = new PointerButton;
    QLineEdit* m_monthEdit = new QLineEdit;
    PointerButton* m_minusDayBtn = new PointerButton;
    PointerButton* m_addDayBtn = new PointerButton;
    QLineEdit* m_dayEdit = new QLineEdit;

    QLabel* m_hourLabel = new QLabel;
    QLabel* m_minuteLabel = new QLabel;
    QLabel* m_yearLabel = new QLabel;
    QLabel* m_monthLabel = new QLabel;
    QLabel* m_dayLabel = new QLabel;

    NavButton* m_acceptBtn = new NavButton;
    NavButton* m_cancelBtn = new NavButton;

    SystemDateFrame* m_ptr;

    void init();
    void initConnection();

    void updateTs() {
        m_title->setText(tr("title"));
        m_subTitle->setText(tr("subTitle"));
        m_autoSyncTimeCheckBox->setText(tr("autoSyncTimeCheckBox"));
        m_hourLabel->setText(tr("Hour"));
        m_minuteLabel->setText(tr("Minute"));
        m_yearLabel->setText(tr("Year"));
        m_monthLabel->setText(tr("Month"));
        m_dayLabel->setText(tr("Day"));
    }

    bool isLeapYear(uint year);
    uint getDaysInMonth(uint year, uint month);
    bool validateHour(const QString& str);
    bool validateMinute(const QString& str);
    bool validateYear(const QString& str);
    bool validateMonth(const QString& str);
    bool validateDay(const QString& str);

    void onYearEditingFinished();
    void onMonthEditingFinished();
    void onDayEditingFinished();
    void onHourEditingFinished();
    void onMinuteEditingFinished();

    void onAddYearBtn();
    void onMinusYearBtn();
    void onAddMonthBtn();
    void onMinusMonthBtn();
    void onMinusDayBtn();
    void onAddDayBtn();
    void onYearMonthDayChanged(QLineEdit* edit, bool add, std::function<bool (const QString&)> validateFunction);

    void onNextButtonClicked();
};

SystemDateFrame::SystemDateFrame(QWidget *parent)
    : QWidget(parent)
    , d_private(new SystemDateFramePrivate(this))
{
    d_private->init();
    d_private->initConnection();
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

void SystemDateFrame::writeConf(bool isLocolTime)
{
    WriteIsLocalTime(isLocolTime);
}

void SystemDateFramePrivate::onYearEditingFinished()
{
    if(!validateYear(m_yearEdit->text())){
        m_yearEdit->setText("1970");
    }
}

void SystemDateFramePrivate::onMonthEditingFinished()
{
    if(!validateMonth(m_monthEdit->text())){
        m_monthEdit->setText("1");
    }
}

void SystemDateFramePrivate::onDayEditingFinished()
{
    if(!validateDay(m_dayEdit->text())){
        m_dayEdit->setText("1");
    }
}

void SystemDateFramePrivate::onHourEditingFinished()
{
    if(!validateHour(m_hourEdit->text())){
        m_hourEdit->setText("0");
    }
}

void SystemDateFramePrivate::onMinuteEditingFinished()
{
    if(!validateMinute(m_minuteEdit->text())){
        m_minuteEdit->setText("0");
    }
}

void SystemDateFramePrivate::onAddYearBtn()
{
    onYearMonthDayChanged(m_yearEdit, true
                          , std::bind(&SystemDateFramePrivate::validateYear, this, std::placeholders::_1));
}

void SystemDateFramePrivate::onMinusYearBtn()
{
    onYearMonthDayChanged(m_yearEdit, false
                          , std::bind(&SystemDateFramePrivate::validateYear, this, std::placeholders::_1));
}

void SystemDateFramePrivate::onAddMonthBtn()
{
    onYearMonthDayChanged(m_monthEdit, true
                          , std::bind(&SystemDateFramePrivate::validateMonth, this, std::placeholders::_1));
}

void SystemDateFramePrivate::onMinusMonthBtn()
{
    onYearMonthDayChanged(m_monthEdit, false
                          , std::bind(&SystemDateFramePrivate::validateMonth, this, std::placeholders::_1));
}

void SystemDateFramePrivate::onAddDayBtn()
{
    onYearMonthDayChanged(m_dayEdit, true
                          , std::bind(&SystemDateFramePrivate::validateDay, this, std::placeholders::_1));
}

void SystemDateFramePrivate::onMinusDayBtn()
{
    onYearMonthDayChanged(m_dayEdit, false
                          , std::bind(&SystemDateFramePrivate::validateDay, this, std::placeholders::_1));
}

void SystemDateFramePrivate::onYearMonthDayChanged(QLineEdit *edit, bool add
                                                   , std::function<bool (const QString &)> validateFunction)
{
    int delta = add ? 1 : -1;
    const QString& str = QString::number(edit->text().toInt() + delta);

    if(validateFunction(str)){
        edit->setText(str);
    }
}

bool SystemDateFramePrivate::isLeapYear(uint year)
{
    return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

uint SystemDateFramePrivate::getDaysInMonth(uint year, uint month)
{
    Q_ASSERT(year >= 1970);
    Q_ASSERT((month >= 1) && (month <= 12));

    switch (month) {
        case 2:{
            if(isLeapYear(year)){
                return 29;
            }
            return 28;
        }
        case 4:
        case 6:
        case 9:
        case 11:{
            return 30;
        }
        default:{
            return 31;
        }
    }
}

bool SystemDateFramePrivate::validateHour(const QString &str)
{
    if(str.toUInt() > 23){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateMinute(const QString &str)
{
    if (str.toUInt() > 59){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateYear(const QString& str)
{
    if(str.toUInt() < 1970){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateMonth(const QString& str)
{
    uint month = str.toUInt();
    if((month < 1) || (month > 12)){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateDay(const QString& str)
{
    uint day = str.toUInt();
    if(day < 1){
        return false;
    }

    uint days = getDaysInMonth(m_yearEdit->text().toUInt(), m_monthEdit->text().toUInt());
    if(day > days){
        return false;
    }

    return true;
}

void SystemDateFramePrivate::onNextButtonClicked()
{
    if(!validateHour(m_hourEdit->text())){
        return;
    }
    if(!validateMinute(m_minuteEdit->text())){
        return;
    }
    if(!validateYear(m_yearEdit->text())){
        return;
    }
    if(!validateMonth(m_monthEdit->text())){
        return;
    }
    if(!validateDay(m_dayEdit->text())){
        return;
    }

    if (m_autoSyncTimeCheckBox->isChecked()) {
        QProcess process;
        process.setProgram("timedatectl set-ntp false");
        process.start();
        process.waitForFinished();

        QString dateTime = QString("'%1-%2-%3 %4:%5:%6'").arg(m_yearEdit->text(), 4, '0')
                .arg(m_monthEdit->text(), 2, '0').arg(m_dayEdit->text(), 2, '0')
                .arg(m_hourEdit->text(), 2, '0').arg(m_minuteEdit->text(), 2, '0').arg("0", 2, '0');
        QString cmd = "timedatectl set-time ";
        cmd.append(dateTime);

        process.setProgram(cmd);
        process.start();
        process.waitForFinished();
    }
    m_ptr->writeConf(m_autoSyncTimeCheckBox->isChecked());

    emit m_ptr->finished();
}

void SystemDateFramePrivate::init()
{
    m_autoSyncTimeCheckBox->setObjectName("autoSyncTimeCheckBox");

    QVBoxLayout* centerLayout = new QVBoxLayout;
    centerLayout->setMargin(0);
    centerLayout->setSpacing(15);
    centerLayout->addSpacing(50);
    centerLayout->addWidget(m_title, 0, Qt::AlignHCenter);
    m_subTitle->setObjectName("subTitleLabel");
    centerLayout->addWidget(m_subTitle, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(40);
    centerLayout->addWidget(m_autoSyncTimeCheckBox, 0, Qt::AlignHCenter);
    centerLayout->addStretch();

    m_autoSyncTimeCheckBox->setCheckState(Qt::Checked);
    m_autoSyncTimeCheckBox->setCheckable(true);
    m_autoSyncTimeCheckBox->setChecked(true);

    m_ptr->setLayout(centerLayout);

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

    m_hourEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_hourEdit->setText("0");
    m_hourEdit->setObjectName("hourEdit");
    m_hourEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hourEdit->setFixedSize(kHourMinuteQLineEditWidth, 36);
    m_hourEdit->setAlignment(Qt::AlignRight);
    QWidget* hourWidget = createWidgetWithBg(m_hourEdit);

    m_minuteEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_minuteEdit->setText("0");
    m_minuteEdit->setObjectName("minuteEdit");
    m_minuteEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_minuteEdit->setFixedSize(kHourMinuteQLineEditWidth, 36);
    m_minuteEdit->setAlignment(Qt::AlignRight);
    QWidget* minuteWidget = createWidgetWithBg(m_minuteEdit);

    m_hourLabel->setObjectName("hourLabel");
    m_hourLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hourLabel->setFixedSize(kHourMinuteQLabelWidth, 36);
    m_minuteLabel->setObjectName("minuteLabel");
    m_minuteLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_minuteLabel->setFixedSize(kHourMinuteQLabelWidth, 36);

    QHBoxLayout* timeLayout = new QHBoxLayout;
    timeLayout->setSpacing(0);
    timeLayout->setMargin(0);
    timeLayout->addStretch();
    timeLayout->addWidget(hourWidget);
    timeLayout->addSpacing(0);
    timeLayout->addWidget(m_hourLabel);
    timeLayout->addSpacing(20);
    timeLayout->addWidget(minuteWidget);
    timeLayout->addSpacing(0);
    timeLayout->addWidget(m_minuteLabel);
    timeLayout->addStretch();

    centerLayout->addSpacing(20);
    centerLayout->addLayout(timeLayout, 0);

    QHBoxLayout* yearLayout = new QHBoxLayout;
    yearLayout->setSpacing(0);
    yearLayout->setMargin(0);

    m_minusYearBtn->setFixedSize(kYearMonthDayQPushButtonWidth, 36);
    m_addYearBtn->setFixedSize(kYearMonthDayQPushButtonWidth, 36);

    m_yearEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,4}")));
    m_yearEdit->setText("1970");
    m_yearEdit->setFixedSize(kYearMonthDayQLineEditWidth, 36);
    m_yearEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_yearEdit->setAlignment(Qt::AlignRight);
    m_yearLabel->setObjectName("yearLabel");
    m_yearLabel->setFixedSize(kYearMonthDayQLabelWidth, 36);
    m_yearLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    yearLayout->addStretch();
    m_minusYearBtn->setObjectName("minusButton");
    yearLayout->addWidget(m_minusYearBtn);
    yearLayout->addWidget(m_yearEdit);
    yearLayout->addWidget(m_yearLabel);
    m_addYearBtn->setObjectName("addButton");
    yearLayout->addWidget(m_addYearBtn);
    yearLayout->addStretch();

    QWidget* yearWidget = new QWidget;
    yearWidget->setObjectName("bgWidget");
    yearWidget->setLayout(yearLayout);
    yearWidget->setFixedSize(kYearMonthDayQWidgetWidth, 36);

    QHBoxLayout* monthLayout = new QHBoxLayout;
    monthLayout->setSpacing(0);
    monthLayout->setMargin(0);

    m_minusMonthBtn->setObjectName("minusButton");
    m_minusMonthBtn->setFixedSize(kYearMonthDayQPushButtonWidth, 36);
    m_addMonthBtn->setObjectName("addButton");
    m_addMonthBtn->setFixedSize(kYearMonthDayQPushButtonWidth, 36);

    m_monthEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_monthEdit->setText("1");
    m_monthEdit->setAlignment(Qt::AlignRight);
    m_monthEdit->setFixedSize(kYearMonthDayQLineEditWidth, 36);
    m_monthEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_monthLabel->setObjectName("monthLabel");
    m_monthLabel->setFixedSize(kYearMonthDayQLabelWidth, 36);
    m_monthLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    monthLayout->addStretch();
    monthLayout->addWidget(m_minusMonthBtn);
    monthLayout->addWidget(m_monthEdit);
    monthLayout->addWidget(m_monthLabel);
    monthLayout->addWidget(m_addMonthBtn);
    monthLayout->addStretch();

    QWidget* monthWidget = new QWidget;
    monthWidget->setObjectName("bgWidget");
    monthWidget->setLayout(monthLayout);
    monthWidget->setFixedSize(kYearMonthDayQWidgetWidth, 36);

    QHBoxLayout* dayLayout = new QHBoxLayout;
    dayLayout->setSpacing(0);
    dayLayout->setMargin(0);

    m_minusDayBtn->setObjectName("minusButton");
    m_addDayBtn->setObjectName("addButton");
    m_minusDayBtn->setFixedSize(kYearMonthDayQPushButtonWidth, 36);
    m_addDayBtn->setFixedSize(kYearMonthDayQPushButtonWidth, 36);

    m_dayEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_dayEdit->setText("1");
    m_dayEdit->setAlignment(Qt::AlignRight);
    m_dayEdit->setFixedSize(kYearMonthDayQLineEditWidth, 36);
    m_dayEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_dayLabel->setObjectName("dayLabel");
    m_dayLabel->setFixedSize(kYearMonthDayQLabelWidth, 36);
    m_dayLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    dayLayout->addStretch();
    dayLayout->addWidget(m_minusDayBtn);
    dayLayout->addWidget(m_dayEdit);
    dayLayout->addWidget(m_dayLabel);
    dayLayout->addWidget(m_addDayBtn);
    dayLayout->addStretch();

    QWidget* dayWidget = new QWidget;
    dayWidget->setObjectName("bgWidget");
    dayWidget->setLayout(dayLayout);
    dayWidget->setFixedSize(kYearMonthDayQWidgetWidth, 36);

    centerLayout->addWidget(yearWidget, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(15);
    centerLayout->addWidget(monthWidget, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(15);
    centerLayout->addWidget(dayWidget, 0, Qt::AlignHCenter);

    centerLayout->addStretch();

    centerLayout->addWidget(m_acceptBtn, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(30);
    centerLayout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);

    connect(m_autoSyncTimeCheckBox, &QCheckBox::clicked, this, [=] {
        m_autoSyncTimeCheckBox->blockSignals(true);
        m_autoSyncTimeCheckBox->setCheckState(Qt::Checked);
        m_autoSyncTimeCheckBox->setChecked(true);
        m_autoSyncTimeCheckBox->blockSignals(false);

        emit m_ptr->cancel();
    });

    setObjectName("systemDateFramePrivate");
    m_ptr->setStyleSheet(ReadFile(":/styles/system_date_frame.css"));
}

void SystemDateFramePrivate::initConnection()
{
    connect(m_yearEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onYearEditingFinished);
    connect(m_monthEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onMonthEditingFinished);
    connect(m_dayEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onDayEditingFinished);
    connect(m_hourEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onHourEditingFinished);
    connect(m_minuteEdit, &QLineEdit::editingFinished, this
            , &SystemDateFramePrivate::onMinuteEditingFinished);

    connect(m_addYearBtn, &QPushButton::clicked, this, &SystemDateFramePrivate::onAddYearBtn);
    connect(m_addMonthBtn, &QPushButton::clicked, this, &SystemDateFramePrivate::onAddMonthBtn);
    connect(m_addDayBtn, &QPushButton::clicked, this, &SystemDateFramePrivate::onAddDayBtn);

    connect(m_minusYearBtn, &QPushButton::clicked, this, &SystemDateFramePrivate::onMinusYearBtn);
    connect(m_minusMonthBtn, &QPushButton::clicked, this, &SystemDateFramePrivate::onMinusMonthBtn);
    connect(m_minusDayBtn, &QPushButton::clicked, this, &SystemDateFramePrivate::onMinusDayBtn);

    connect(m_acceptBtn, &NavButton::clicked, this, &SystemDateFramePrivate::onNextButtonClicked);
    connect(m_cancelBtn, &NavButton::clicked, m_ptr, &SystemDateFrame::cancel);
}

}

#include "systemdateframe.moc"
