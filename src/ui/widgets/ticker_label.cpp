/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "ticker_label.h"

#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace installer {

TickerLabel::TickerLabel(QWidget *parent):
    QLabel(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(300);
    connect(m_timer, &QTimer::timeout, this, &TickerLabel::updateText);
}

void TickerLabel::setText(const QString &text)
{
    m_showText = text;
    QLabel::setText(m_showText);
}

QString TickerLabel::text() const
{
    return m_showText;
}

void TickerLabel::start()
{
    if (this->fontMetrics().width(m_showText) > width() && !m_timer->isActive()) {
        m_timer->start();
    }
}

void TickerLabel::stop()
{
    m_pos = 0;
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    QLabel::setText(m_showText);
}

void TickerLabel::updateText()
{
    QString show = m_showText.mid(m_pos);
    if (fontMetrics().width(show) > width()) {
        m_pos++;
    }

    QLabel::setText(m_showText.mid(m_pos));
}

}
