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
#include <DCheckBox>

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {
    int kSystemDateTimeFrameWidth = 691;
    int kMonthDayHourMinuteQLineEditWidth = 38;
    int kDateTimeLabelWidth = 45;
    int kYearQLineEditWidth = 55;
    int kMaxIncrementYear = 30;

    int kDateTimeItemInnerSpacing = 1;
    int kDateTimeItemExternalSpacing = 2;

    int kDateTimeFontSize = 10; // 10pt
}

class SystemDateFramePrivate : public QObject{
    Q_OBJECT
public:
    SystemDateFramePrivate(SystemDateFrame* parent) : q_ptr(qobject_cast<SystemDateFrame*>(parent)) {}

    DCheckBox *m_setDateTimeCheckBox = new DCheckBox;
    TimeDateLineEdit* m_hourEdit = new TimeDateLineEdit;
    TimeDateLineEdit* m_minuteEdit = new TimeDateLineEdit;
    TimeDateLineEdit* m_yearEdit = new TimeDateLineEdit;
    TimeDateLineEdit* m_monthEdit = new TimeDateLineEdit;
    TimeDateLineEdit* m_dayEdit = new TimeDateLineEdit;

    QLabel* m_hourLabel = new QLabel;
    QLabel* m_minuteLabel = new QLabel;
    QLabel* m_yearLabel = new QLabel;
    QLabel* m_monthLabel = new QLabel;
    QLabel* m_dayLabel = new QLabel;

    int m_maxYear = 9999;

    SystemDateFrame* q_ptr;
    Q_DECLARE_PUBLIC(SystemDateFrame)

    void initUI();
    void initConnection();

    void updateTs() {
        m_setDateTimeCheckBox->setText(::QObject::tr("Change time"));
        m_hourLabel->setText(::QObject::tr("hr"));
        m_minuteLabel->setText(::QObject::tr("min"));
        m_yearLabel->setText(::QObject::tr("Y"));
        m_monthLabel->setText(::QObject::tr("M"));
        m_dayLabel->setText(::QObject::tr("D"));
    }

    void initDateTime();
    bool isLeapYear(uint year) const;
    uint getDaysInMonth(uint year, uint month) const;
    bool validateHour(const QString& str) const;
    bool check24Hour(const QString &str) const;
    bool validateMinute(const QString& str) const;
    bool validateYear(const QString& str) const;
    bool validateMonth(const QString& str) const;
    bool validateDay(const QString& str) const;

    void onYearEditingFinished();
    void onMonthEditingFinished();
    void onDayEditingFinished();
    void onHourEditingFinished();
    void onMinuteEditingFinished();

    void onYearMonthDayChanged(TimeDateLineEdit* edit, bool add);
    void autoAdjustDay();
};

SystemDateFrame::SystemDateFrame(QWidget *parent)
    : QWidget(parent)
    , d_private(new SystemDateFramePrivate(this))
{
    d_private->initUI();
    d_private->initDateTime();
    d_private->initConnection();
    d_private->updateTs();

    this->setFocusProxy(d_private->m_setDateTimeCheckBox);
}

SystemDateFrame::~SystemDateFrame()
{
}

QString SystemDateFrame::timedate() const {
  return QString("%1-%2-%3 %4:%5:%6")
      .arg(d_private->m_yearEdit->text(), 4, '0')
      .arg(d_private->m_monthEdit->text(), 2, '0')
      .arg(d_private->m_dayEdit->text(), 2, '0')
      .arg(d_private->m_hourEdit->text(), 2, '0')
      .arg(d_private->m_minuteEdit->text(), 2, '0')
      .arg("0", 2, '0');
}

bool SystemDateFrame::isEnabled() const {
    Q_D(const SystemDateFrame);

    return d->m_setDateTimeCheckBox->isChecked();
}

bool SystemDateFrame::event(QEvent *event)
{
    Q_D(SystemDateFrame);

    if (event->type() == QEvent::LanguageChange) {
        d->updateTs();
    }

    return QWidget::event(event);
}

bool SystemDateFrame::validateTimeDate() const
{
    Q_D(const SystemDateFrame);

    if(!d->validateHour(d->m_hourEdit->text())){
        return false;
    }
    if(!d->validateMinute(d->m_minuteEdit->text())){
        return false;
    }
    if(!d->validateYear(d->m_yearEdit->text())){
        return false;
    }
    if(!d->validateMonth(d->m_monthEdit->text())){
        return false;
    }
    if(!d->validateDay(d->m_dayEdit->text())){
        return false;
    }

    if(d->m_hourEdit->text().toInt() == 0
            && d->m_minuteEdit->text().toInt() == 0
            && d->m_yearEdit->text().toInt() == 1970
            && d->m_monthEdit->text().toInt() == 1
            && d->m_dayEdit->text().toInt() == 1){
        return false;
    }

    return true;
}

void SystemDateFramePrivate::onYearEditingFinished()
{
    Q_Q(SystemDateFrame);
    q->setFocus();
    if(!validateYear(m_yearEdit->text())){
        m_yearEdit->setText(QString::number(QDate::currentDate().year()));
    }

    autoAdjustDay();
}

void SystemDateFramePrivate::onMonthEditingFinished()
{
    Q_Q(SystemDateFrame);
    q->setFocus();
    if(!validateMonth(m_monthEdit->text())){
        m_monthEdit->setText(QString::number(QDate::currentDate().month()));
    }

    autoAdjustDay();
}

void SystemDateFramePrivate::onDayEditingFinished()
{
    Q_Q(SystemDateFrame);
    q->setFocus();
    if(!validateDay(m_dayEdit->text())){
        m_dayEdit->setText(QString::number(QDate::currentDate().day()));
    }
}

void SystemDateFramePrivate::onHourEditingFinished()
{
    Q_Q(SystemDateFrame);
    q->setFocus();
    if (!check24Hour(m_hourEdit->text())) {
        m_hourEdit->setText(QString::number(0));
    } else if(!validateHour(m_hourEdit->text())){
        m_hourEdit->setText(QString::number(QTime::currentTime().hour()));
    }
}

void SystemDateFramePrivate::onMinuteEditingFinished()
{
    Q_Q(SystemDateFrame);
    q->setFocus();
    if(!validateMinute(m_minuteEdit->text())){
        m_minuteEdit->setText(QString::number(QTime::currentTime().minute()));
    }
}

void SystemDateFramePrivate::onYearMonthDayChanged(TimeDateLineEdit *edit, bool add)
{
    int delta = add ? 1 : -1;
    int val = edit->text().toInt() + delta;
    int first;
    int count;

    if(edit == m_yearEdit){
        first = 1970;
        count = m_maxYear - 1970 + 1;
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
        m_dayEdit->setText(QString::number(QDate::currentDate().day()));
    }
}

bool SystemDateFramePrivate::isLeapYear(uint year) const
{
    return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

uint SystemDateFramePrivate::getDaysInMonth(uint year, uint month) const
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

bool SystemDateFramePrivate::validateHour(const QString &str) const
{
    if(str.isEmpty()){
        return false;
    }

    if(str.toUInt() > 23){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::check24Hour(const QString &str) const
{
    if(str.isEmpty()){
        return false;
    }

    if(str.toUInt() == 24){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateMinute(const QString &str) const
{
    if(str.isEmpty()){
        return false;
    }

    if (str.toUInt() > 59){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateYear(const QString& str) const
{
    if(str.isEmpty()){
        return false;
    }

    if(str.toUInt() < 1970){
        return false;
    }

    if(str.toUInt() > m_maxYear){
        return false;
    }

    return true;
}

bool SystemDateFramePrivate::validateMonth(const QString& str) const
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

bool SystemDateFramePrivate::validateDay(const QString& str) const
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

void SystemDateFrame::timeDateSetFinished()
{
    Q_D(SystemDateFrame);

#if 0 // 更改到timezone_frame中writeConf完成
    QProcess process;
    qDebug() << process.execute("timedatectl", QStringList() << "set-ntp" << "false");

    const QString dateTime = m_ptr->timedate();

    qDebug() << process.execute("timedatectl", QStringList() << "set-time" << dateTime);
    WriteIsLocalTime(true);
    WriteIsLocalTimeForce(true);
#endif

    emit finished();
}

bool SystemDateFrame::focusSwitch()
{
    return true;
}

bool SystemDateFrame::doSpace()
{
    if (d_private->m_setDateTimeCheckBox->hasFocus()) {
        d_private->m_setDateTimeCheckBox->setChecked(!d_private->m_setDateTimeCheckBox->isChecked());
    }
    return true;
}

bool SystemDateFrame::doSelect()
{
    return true;
}

bool SystemDateFrame::directionKey(int keyvalue)
{
    switch (keyvalue) {
    case Qt::Key_Up: {
            if (d_private->m_hourEdit->hasFocus()) {

            } else if (d_private->m_minuteEdit->hasFocus()) {

            } else if (d_private->m_yearEdit->hasFocus()) {

            } else if (d_private->m_monthEdit->hasFocus()) {

            } else if (d_private->m_dayEdit->hasFocus()) {

            }
        }
        break;
    case Qt::Key_Down: {

        }
        break;
    case Qt::Key_Left: {
            if (d_private->m_setDateTimeCheckBox->hasFocus()) {
            } else if (d_private->m_yearEdit->hasFocus()) {
                d_private->m_yearEdit->clearFocus();
                d_private->m_setDateTimeCheckBox->setFocus();
            } else if (d_private->m_monthEdit->hasFocus()) {
                d_private->m_monthEdit->clearFocus();
                d_private->m_yearEdit->setFocus();
            } else if (d_private->m_dayEdit->hasFocus()) {
                d_private->m_dayEdit->clearFocus();
                d_private->m_monthEdit->setFocus();
            } else if (d_private->m_hourEdit->hasFocus()) {
                d_private->m_hourEdit->clearFocus();
                d_private->m_dayEdit->setFocus();
            } else if (d_private->m_minuteEdit->hasFocus()) {
                d_private->m_minuteEdit->clearFocus();
                d_private->m_hourEdit->setFocus();
            }
        }
        break;
    case Qt::Key_Right: {
            if (d_private->m_setDateTimeCheckBox->hasFocus()) {
                d_private->m_setDateTimeCheckBox->clearFocus();
                d_private->m_yearEdit->setFocus();
            } else if (d_private->m_yearEdit->hasFocus()) {
                d_private->m_yearEdit->clearFocus();
                d_private->m_monthEdit->setFocus();
            } else if (d_private->m_monthEdit->hasFocus()) {
                d_private->m_monthEdit->clearFocus();
                d_private->m_dayEdit->setFocus();
            } else if (d_private->m_dayEdit->hasFocus()) {
                d_private->m_dayEdit->clearFocus();
                d_private->m_hourEdit->setFocus();
            } else if (d_private->m_hourEdit->hasFocus()) {
                d_private->m_hourEdit->clearFocus();
                d_private->m_minuteEdit->setFocus();
            } else if (d_private->m_minuteEdit->hasFocus()) {
            }
        }
        break;
    }

    return true;
}

void SystemDateFramePrivate::initUI()
{
    m_setDateTimeCheckBox->setCheckable(true);
    m_setDateTimeCheckBox->setChecked(false);
    //m_setDateTimeCheckBox->setFocusPolicy(Qt::NoFocus);

    m_hourEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_hourEdit->setObjectName("hourEdit");
    m_hourEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hourEdit->setFixedSize(kMonthDayHourMinuteQLineEditWidth, 36);
    m_hourEdit->setAlignment(Qt::AlignHCenter);
    m_hourEdit->setContextMenuPolicy(Qt::NoContextMenu);
    //m_hourEdit->setFocusPolicy(Qt::NoFocus);

    m_minuteEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_minuteEdit->setObjectName("minuteEdit");
    m_minuteEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_minuteEdit->setFixedSize(kMonthDayHourMinuteQLineEditWidth, 36);
    m_minuteEdit->setAlignment(Qt::AlignHCenter);
    m_minuteEdit->setContextMenuPolicy(Qt::NoContextMenu);
    //m_minuteEdit->setFocusPolicy(Qt::NoFocus);

    m_hourLabel->setObjectName("hourLabel");
    m_hourLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hourLabel->setFixedSize(kDateTimeLabelWidth, 36);
    m_minuteLabel->setObjectName("minuteLabel");
    m_minuteLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_minuteLabel->setFixedSize(kDateTimeLabelWidth, 36);

    m_yearEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,4}")));
    m_yearEdit->setFixedSize(kYearQLineEditWidth, 36);
    m_yearEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_yearEdit->setAlignment(Qt::AlignHCenter);
    m_yearEdit->setContextMenuPolicy(Qt::NoContextMenu);
    //m_yearEdit->setFocusPolicy(Qt::NoFocus);

    m_yearLabel->setObjectName("yearLabel");
    m_yearLabel->setFixedSize(kDateTimeLabelWidth, 36);
    m_yearLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_monthEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_monthEdit->setAlignment(Qt::AlignHCenter);
    m_monthEdit->setFixedSize(kMonthDayHourMinuteQLineEditWidth, 36);
    m_monthEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_monthEdit->setContextMenuPolicy(Qt::NoContextMenu);
    //m_monthEdit->setFocusPolicy(Qt::NoFocus);

    m_monthLabel->setObjectName("monthLabel");
    m_monthLabel->setFixedSize(kDateTimeLabelWidth, 36);
    m_monthLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_dayEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}")));
    m_dayEdit->setAlignment(Qt::AlignHCenter);
    m_dayEdit->setFixedSize(kMonthDayHourMinuteQLineEditWidth, 36);
    m_dayEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_dayEdit->setContextMenuPolicy(Qt::NoContextMenu);
    //m_dayEdit->setFocusPolicy(Qt::NoFocus);

    m_dayLabel->setObjectName("dayLabel");
    m_dayLabel->setFixedSize(kDateTimeLabelWidth, 36);
    m_dayLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(m_setDateTimeCheckBox, 0, Qt::AlignLeft);
    mainLayout->addStretch();
    mainLayout->addWidget(m_yearEdit);
    mainLayout->addSpacing(kDateTimeItemInnerSpacing);
    mainLayout->addWidget(m_yearLabel);
    mainLayout->addSpacing(kDateTimeItemExternalSpacing);
    mainLayout->addWidget(m_monthEdit);
    mainLayout->addSpacing(kDateTimeItemInnerSpacing);
    mainLayout->addWidget(m_monthLabel);
    mainLayout->addSpacing(kDateTimeItemExternalSpacing);
    mainLayout->addWidget(m_dayEdit);
    mainLayout->addSpacing(kDateTimeItemInnerSpacing);
    mainLayout->addWidget(m_dayLabel);
    mainLayout->addSpacing(kDateTimeItemExternalSpacing);
    mainLayout->addWidget(m_hourEdit);
    mainLayout->addSpacing(kDateTimeItemInnerSpacing);
    mainLayout->addWidget(m_hourLabel);
    mainLayout->addSpacing(kDateTimeItemExternalSpacing);
    mainLayout->addWidget(m_minuteEdit);
    mainLayout->addSpacing(kDateTimeItemInnerSpacing);
    mainLayout->addWidget(m_minuteLabel);
    q_ptr->setLayout(mainLayout);

    QFont font;
    font.setPixelSize(kDateTimeFontSize);
    m_hourEdit->setFont(font);
    m_hourLabel->setFont(font);
    m_minuteEdit->setFont(font);
    m_minuteLabel->setFont(font);
    m_yearEdit->setFont(font);
    m_yearLabel->setFont(font);
    m_monthEdit->setFont(font);
    m_monthLabel->setFont(font);
    m_dayEdit->setFont(font);
    m_dayLabel->setFont(font);
    m_setDateTimeCheckBox->setFont(font);

    setObjectName("systemDateFramePrivate");
    q_ptr->setMaximumWidth(kSystemDateTimeFrameWidth);
    q_ptr->setMaximumHeight(40);
}

void SystemDateFramePrivate::initConnection()
{
    Q_Q(SystemDateFrame);

    connect(m_setDateTimeCheckBox, &DCheckBox::stateChanged, this, [=] {
        m_hourEdit->setEnabled(m_setDateTimeCheckBox->isChecked());
        m_minuteEdit->setEnabled(m_setDateTimeCheckBox->isChecked());
        m_yearEdit->setEnabled(m_setDateTimeCheckBox->isChecked());
        m_monthEdit->setEnabled(m_setDateTimeCheckBox->isChecked());
        m_dayEdit->setEnabled(m_setDateTimeCheckBox->isChecked());
    });
    emit m_setDateTimeCheckBox->stateChanged(m_setDateTimeCheckBox->checkState());

    connect(m_yearEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onYearEditingFinished);
    connect(m_yearEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onYearEditingFinished);
    connect(m_yearEdit, &TimeDateLineEdit::selectionChanged, [this]{m_yearEdit->setFocus();});

    connect(m_monthEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onMonthEditingFinished);
    connect(m_monthEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onMonthEditingFinished);
    connect(m_monthEdit, &TimeDateLineEdit::selectionChanged, [this]{m_monthEdit->setFocus();});

    connect(m_dayEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onDayEditingFinished);
    connect(m_dayEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onDayEditingFinished);
    connect(m_dayEdit, &TimeDateLineEdit::selectionChanged, [this]{m_dayEdit->setFocus();});

    connect(m_hourEdit, &QLineEdit::editingFinished, this,
            &SystemDateFramePrivate::onHourEditingFinished);
    connect(m_hourEdit, &TimeDateLineEdit::lostFocus, this,
            &SystemDateFramePrivate::onHourEditingFinished);
    connect(m_hourEdit, &TimeDateLineEdit::selectionChanged, [this]{m_hourEdit->setFocus();});

    connect(m_minuteEdit, &QLineEdit::editingFinished, this
            , &SystemDateFramePrivate::onMinuteEditingFinished);
    connect(m_minuteEdit, &TimeDateLineEdit::lostFocus, this
            , &SystemDateFramePrivate::onMinuteEditingFinished);
    connect(m_minuteEdit, &TimeDateLineEdit::selectionChanged, [this]{m_minuteEdit->setFocus();});
}

void SystemDateFramePrivate::initDateTime()
{
    const QDateTime& currentDateTime = QDateTime::currentDateTime();
    const QDate& date = currentDateTime.date();
    const QTime& time = currentDateTime.time();

    m_yearEdit->setText(QString::number(date.year()));
    m_maxYear = date.year() + kMaxIncrementYear;
    m_monthEdit->setText(QString::number(date.month()));
    m_dayEdit->setText(QString::number(date.day()));
    m_hourEdit->setText(QString::number(time.hour()));
    m_minuteEdit->setText(QString::number(time.minute()));
}

}

#include "systemdateframe.moc"
