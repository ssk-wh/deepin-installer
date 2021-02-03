/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#include "ddropdown.h"
#include "ui/utils/widget_util.h"

#include <QDebug>
#include <QMenu>
#include <QHBoxLayout>
#include <QStyle>
#include <QMouseEvent>
#include <QLabel>

#include <DThemeManager>

DWIDGET_USE_NAMESPACE

class DDropdownPrivate
{
public:
    enum class ButtonStatus {
        Normal,
        Hover,
        Press
    };

    DDropdownPrivate(DDropdown *parent)
        : q_ptr(parent)
        , m_state(ButtonStatus::Normal)
    {}

    void updatePic();

    DDropdown *q_ptr;
    Q_DECLARE_PUBLIC(DDropdown)

    QMenu       *menu       = nullptr;
    QLabel      *text       = nullptr;
    QLabel      *dropdown   = nullptr;
    ButtonStatus m_state    = ButtonStatus::Normal;
};

DDropdown::DDropdown(QWidget *parent)
    : QFrame(parent)
    , d_ptr(new DDropdownPrivate(this))
{
    Q_D(DDropdown);

    setObjectName("DDropdown");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    d->menu = new QMenu;
    d->text = new QLabel("undefined");
    d->text->setObjectName("DDropdownText");
    d->dropdown = new QLabel;
    d->dropdown->setObjectName("DDropdownIcon");
    d->dropdown->setPixmap(installer::renderPixmap(":/images/dropdown_arrow-normal.svg"));
    d->dropdown->setFixedSize(8, 5);

    auto wrapLayout = new QHBoxLayout;
    wrapLayout->setContentsMargins(0, 0, 0, 0);
    wrapLayout->setSpacing(0);
    wrapLayout->addWidget(d->text, 0, Qt::AlignVCenter);
    wrapLayout->addSpacing(8);
    wrapLayout->addWidget(d->dropdown, 0, Qt::AlignVCenter);

    auto wrapWidget = new QWidget;
    wrapWidget->setLayout(wrapLayout);

    layout->addStretch();
    layout->addWidget(wrapWidget, 0, Qt::AlignVCenter);
    layout->addStretch();

    setFixedWidth(130);

    connect(d->menu, &QMenu::triggered, this, [ = ](QAction * action) {
        d->text->setText(action->text());
        Q_EMIT this->triggered(action);
    });

    connect(this, &DDropdown::requestContextMenu,
    this, [ = ]() {
        auto center = this->mapToGlobal(this->rect().topLeft());
        center.setX(center.x() - 16);
        center.setY(center.y() + this->height() + 5);
        d->menu->move(center);
        d->menu->exec();
    });
}

DDropdown::~DDropdown()
{

}

QList<QAction *> DDropdown::actions() const
{
    Q_D(const DDropdown);
    return d->menu->actions();
}

void DDropdown::setText(const QString &text)
{
    Q_D(DDropdown);
    d->text->setText(text);
}

void DDropdown::setCurrentAction(QAction *action)
{
    Q_D(DDropdown);
    if (action) {
        for (auto action : d->menu->actions()) {
            action->setChecked(false);
        }
        d->text->setText(action->text());
        action->setChecked(true);
    } else {
        for (auto action : d->menu->actions()) {
            action->setChecked(false);
        }
    }
}

QAction *DDropdown::addAction(const QString &item, const QVariant &var)
{
    Q_D(DDropdown);
    auto action = d->menu->addAction(item);
    action->setData(var);
    action->setCheckable(true);
    return action;
}

void DDropdown::enterEvent(QEvent *event)
{
    Q_D(DDropdown);
    d->m_state = DDropdownPrivate::ButtonStatus::Hover;
    d->updatePic();

    this->style()->unpolish(this);
    this->style()->polish(this);
    update();

    QFrame::enterEvent(event);
}

void DDropdown::leaveEvent(QEvent *event)
{
    Q_D(DDropdown);
    d->m_state = DDropdownPrivate::ButtonStatus::Normal;
    d->updatePic();

    this->style()->unpolish(this);
    this->style()->polish(this);
    update();

    QFrame::leaveEvent(event);
}

void DDropdown::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_D(DDropdown);
        d->m_state = DDropdownPrivate::ButtonStatus::Press;
        d->updatePic();
    }

    QFrame::mousePressEvent(event);
}

void DDropdown::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DDropdown);
    if (d->m_state == DDropdownPrivate::ButtonStatus::Press && rect().contains(event->pos())) {
        d->m_state = DDropdownPrivate::ButtonStatus::Hover;
        d->updatePic();
    }

    if (event->button() == Qt::LeftButton) {
        Q_EMIT requestContextMenu();
    }

    QFrame::mouseReleaseEvent(event);
}

void DDropdownPrivate::updatePic()
{
    switch(m_state){
    case ButtonStatus::Normal:
        dropdown->setPixmap(installer::renderPixmap(":/images/dropdown_arrow-normal.svg"));
        break;
    case ButtonStatus::Hover:
        dropdown->setPixmap(installer::renderPixmap(":/images/dropdown_arrow-hover.svg"));
        break;
    case ButtonStatus::Press:
        dropdown->setPixmap(installer::renderPixmap(":/images/dropdown_arrow-press.svg"));
        break;
    default:
        qCritical() << "Invalid DDropdown state";
        break;
    }
}
