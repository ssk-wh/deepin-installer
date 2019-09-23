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
#include "ui/widgets/time_date_line_edit.h"
#include "ui/widgets/comment_label.h"
#include "service/settings_manager.h"
#include "ui/frames/consts.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QEvent>
#include <QChar>
#include <QProcess>
#include <QRegExpValidator>
#include <QDebug>
#include <functional>
#include <QDateTime>

namespace installer {

namespace {
    int kHourMinuteQLineEditWidth = 120;
    int kHourMinuteQLabelWidth = 40;
    int kYearMonthDayQPushButtonWidth = 35;
    int kYearMonthDayQLineEditWidth = 160;
    int kYearMonthDayQLabelWidth = 110;
    int kYearMonthDayQWidgetWidth = 340;
}

class SystemDateFramePrivate : public QWidget{
    Q_OBJECT
public:
    SystemDateFramePrivate(SystemDateFrame* qq) : m_ptr(qq) {}

    QLabel* m_title = new TitleLabel(tr("Time setting"));
    CommentLabel* m_commentLabel = new CommentLabel(tr("Change date and time as you want"));

    TimeDateLineEdit* m_hourEdit = new TimeDateLineEdit(this);
    TimeDateLineEdit* m_minuteEdit = new TimeDateLineEdit(this);
    PointerButton* m_addYearBtn   = new PointerButton;
    PointerButton* m_minusYearBtn = new PointerButton;
    TimeDateLineEdit* m_yearEdit = new TimeDateLineEdit(this);
    PointerButton* m_addMonthBtn = new PointerButton;
    PointerButton* m_minusMonthBtn = new PointerButton;
    TimeDateLineEdit* m_monthEdit = new TimeDateLineEdit(this);
    PointerButton* m_minusDayBtn = new PointerButton;
    PointerButton* m_addDayBtn = new PointerButton;
    TimeDateLineEdit* m_dayEdit = new TimeDateLineEdit(this);

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
        m_title->setText(tr("Time setting"));
        m_commentLabel->setText(tr("Change date and time as you want"));
        m_hourLabel->setText(tr("Hour"));
        m_minuteLabel->setText(tr("Minute"));
        m_yearLabel->setText(tr("Year"));
        m_monthLabel->setText(tr("Month"));
        m_dayLabel->setText(tr("Day"));
        m_acceptBtn->setText(tr("Accept"));
        m_cancelBtn->setText(tr("Cancel"));
    }

    void initDateTime();
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
    void onYearMonthDayChanged(TimeDateLineEdit* edit, bool add);
    void autoAdjustDay();

    void onNextButtonClicked();
};

SystemDateFrame::SystemDateFrame(QWidget *parent)
    : QWidget(parent)
    , d_private(new SystemDateFramePrivate(this))
{
    d_private->init();
    d_private->initDateTime();
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
    if (event->type() == QEvent::Show){
        d->m_acceptBtn->setFocus();
    }

    return QWidget::event(event);
}

void SystemDateFramePrivate::onYearEditingFinished()
{
    if(!validateYear(m_yearEdit->text())){
        m_yearEdit->setText(QString::number(QDate::currentDate().year()));
    }

    autoAdjustDay();
}

void SystemDateFramePrivate::onMonthEditingFinished()
{
    if(!validateMonth(m_monthEdit->text())){
        m_monthEdit->setText(QString::number(QDate::currentDate().month()));
    }

    autoAdjustDay();
}

void SystemDateFramePrivate::onDayEditingFinished()
{
    if(!validateDay(m_dayEdit->text())){
        m_dayEdit->setText(QString::number(QDate::currentDate().day()));
    }
}

void SystemDateFramePrivate::onHourEditingFinished()
{
    if(!validateHour(m_hourEdit->text())){
        m_hourEdit->setText(QString::number(QTime::currentTime().hour()));
    }
}

void SystemDateFramePrivate::onMinuteEditingFinished()
{
    if(!validateMinute(m_minuteEdit->text())){
        m_minuteEdit->setText(QString::number(QTime::currentTime().minute()));
    }
}

void SystemDateFramePrivate::onAddYearBtn()
{
    onYearMonthDayChanged(m_yearEdit, true);
}

void SystemDateFramePrivate::onMinusYearBtn()
{
    onYearMonthDayChanged(m_yearEdit, false);
}

void SystemDateFramePrivate::onAddMonthBtn()
{
    onYearMonthDayChanged(m_monthEdit, true);
}

void SystemDateFramePrivate::onMinusMonthBtn()
{
    onYearMonthDayChanged(m_monthEdit, false);
}

void SystemDateFramePrivate::onAddDayBtn()
{
    onYearMonthDayChanged(m_dayEdit, true);
}

void SystemDateFramePrivate::onMinusDayBtn()
{
    onYearMonthDayChanged(m_dayEdit, false);
}

void SystemDateFramePrivate::onYearMonthDayChanged(TimeDateLineEdit *edit, bool add)
{
    int delta = add ? 1 : -1;
    int val = edit->text().toInt() + delta;
    int first;
    int count;

    if(edit == m_yearEdit){
        first = 1970;
        count = 9999 - 1970 + 1;
    }
    else if (edit == m_monthEdit) {
        first = 1;
        count = 12;
    }
    else {
        Q_ASSERT(edit == m_dayEdit);
        first = 1;
        count = static_cast<int>(getDaysInMonth(m_yearEdit->text().toUInt(), m_monthEdit->text().toUInt()));
    }

    val = (val - first + count) % count + first;
    edit->setText(QString::number(val));

    if((edit == m_yearEdit) || (edit == m_monthEdit)){
        autoAdjustDay();
    }
}

void SystemDateFramePrivate::autoAdjustDay()
{
    uint days = getDaysInMonth(m_yearEdit->text().toUInt(), m_monthEdit->text().toUInt());
    uint day = m_dayEdit->text().toUInt();
    if(day > days){
        m_dayEdit->setText(QString::number(days));
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
    if(str.isEmpty()){
        return false;
    }

    if(str.toUInt() > 23){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateMinute(const QString &str)
{
    if(str.isEmpty()){
        return false;
    }

    if (str.toUInt() > 59){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateYear(const QString& str)
{
    if(str.isEmpty()){
        return false;
    }

    if(str.toUInt() < 1970){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateMonth(const QString& str)
{
    if(str.isEmpty()){
        return false;
    }

    uint month = str.toUInt();
    if((month < 1) || (month > 12)){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateDay(const QString& str)
{
    if(str.isEmpty()){
        return false;
    }

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

    if(m_hourEdit->text().toInt() == 0
            && m_minuteEdit->text().toInt() == 0
            && m_yearEdit->text().toInt() == 1970
            && m_monthEdit->text().toInt() == 1
            && m_dayEdit->text().toInt() == 1){
        return;
    }

    QProcess process;
    qDebug() << process.execute("timedatectl", QStringList() << "set-ntp" << "false");

    QString dateTime = QString("%1-%2-%3 %4:%5:%6").arg(m_yearEdit->text(), 4, '0')
            .arg(m_monthEdit->text(), 2, '0').arg(m_dayEdit->text(), 2, '0')
            .arg(m_hourEdit->text(), 2, '0').arg(m_minuteEdit->text(), 2, '0').arg("0", 2, '0');

    qDebug() << process.execute("timedatectl", QStringList() << "set-time" << dateTime);
    WriteIsLocalTime(true);

    emit m_ptr->finished();
}

void SystemDateFramePrivate::init()
{
    QVBoxLayout* centerLayout = new QVBoxLayout;
    centerLayout->setMargin(0);
    centerLayout->setSpacing(kMainLayoutSpacing);
    centerLayout->addSpacing(50);
    centerLayout->addWidget(m_title, 0, Qt::AlignHCenter);
    centerLayout->addWidget(m_commentLabel, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(50);
    centerLayout->addStretch();

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
    m_hourEdit->setObjectName("hourEdit");
    m_hourEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hourEdit->setFixedSize(kHourMinuteQLineEditWidth, 36);
    m_hourEdit->setAlignment(Qt::AlignRight);
    m_hourEdit->setContextMenuPolicy(Qt::NoContextMenu);
    QWidget* hourWidget = createWidgetWithBg(m_hourEdit);

    m_minuteEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_minuteEdit->setObjectName("minuteEdit");
    m_minuteEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_minuteEdit->setFixedSize(kHourMinuteQLineEditWidth, 36);
    m_minuteEdit->setAlignment(Qt::AlignRight);
    m_minuteEdit->setContextMenuPolicy(Qt::NoContextMenu);
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
    m_yearEdit->setFixedSize(kYearMonthDayQLineEditWidth, 36);
    m_yearEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_yearEdit->setAlignment(Qt::AlignRight);
    m_yearEdit->setContextMenuPolicy(Qt::NoContextMenu);
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
    m_monthEdit->setAlignment(Qt::AlignRight);
    m_monthEdit->setFixedSize(kYearMonthDayQLineEditWidth, 36);
    m_monthEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_monthEdit->setContextMenuPolicy(Qt::NoContextMenu);
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
    m_dayEdit->setAlignment(Qt::AlignRight);
    m_dayEdit->setFixedSize(kYearMonthDayQLineEditWidth, 36);
    m_dayEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_dayEdit->setContextMenuPolicy(Qt::NoContextMenu);
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

    setObjectName("systemDateFramePrivate");
    m_ptr->setStyleSheet(ReadFile(":/styles/system_date_frame.css"));
}

void SystemDateFramePrivate::initConnection()
{
    connect(m_yearEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onYearEditingFinished);
    connect(m_yearEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onYearEditingFinished);

    connect(m_monthEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onMonthEditingFinished);
    connect(m_monthEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onMonthEditingFinished);

    connect(m_dayEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onDayEditingFinished);
    connect(m_dayEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onDayEditingFinished);

    connect(m_hourEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onHourEditingFinished);
    connect(m_hourEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onHourEditingFinished);

    connect(m_minuteEdit, &QLineEdit::editingFinished, this
            , &SystemDateFramePrivate::onMinuteEditingFinished);
    connect(m_minuteEdit, &TimeDateLineEdit::lostFocus, this
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

void SystemDateFramePrivate::initDateTime()
{
    const QDateTime& currentDateTime = QDateTime::currentDateTime();
    const QDate& date = currentDateTime.date();
    const QTime& time = currentDateTime.time();

    m_yearEdit->setText(QString::number(date.year()));
    m_monthEdit->setText(QString::number(date.month()));
    m_dayEdit->setText(QString::number(date.day()));
    m_hourEdit->setText(QString::number(time.hour()));
    m_minuteEdit->setText(QString::number(time.minute()));
}

}

#include "systemdateframe.moc"
