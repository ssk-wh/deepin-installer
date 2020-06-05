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

#ifndef TICKER_LABEL_H
#define TICKER_LABEL_H

#include <QLabel>

class QTimer;

namespace installer {

class TickerLabel : public QLabel
{
    Q_OBJECT
public:
    TickerLabel(QWidget *parent = nullptr);

public:
    QString text() const;
    void setText(const QString &text);
    void start();
    void stop();

protected:
    void updateText();

private:
    int     m_pos = 0;
    QString m_showText;
    QTimer *m_timer = nullptr;
};
}

#endif // TICKER_LABEL_H
