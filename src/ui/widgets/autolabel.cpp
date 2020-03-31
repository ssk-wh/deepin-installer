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
 * along with this program.  If not, see .
 */

#include "autolabel.h"


AutoLabel::AutoLabel(QWidget *parent, Qt::WindowFlags f):
    QLabel (parent, f)
{

}

AutoLabel::AutoLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    this->setText(text);
}

void AutoLabel::setText(const QString &text)
{
    this->setInputText(text);

    if (text.isEmpty())
    {
        return QLabel::setText(text);
    }

    // 计算一行显示的字符的size
    int ch_win = QLabel::fontMetrics().width(text.at(0));
    int show_wid = this->width() - 2 * this->margin();
    int show_size = show_wid / ch_win;

    QStringList setList;
    QStringList lineList = text.split("\n");

    // 对输入的每一行就行处理，如果长度超出每一行的size，则在超出的每部分上插入"\n"
    foreach (QString line, lineList) {
        if (line.size() <= show_size) {
            setList.append(line);

        } else {
            for (int pos = line.size() / show_size; pos > 0; pos--) {
                line.insert(pos * show_size, "\n");
            }

            setList.append(line);
        }
    }

    return QLabel::setText(setList.join("\n"));
}

QString AutoLabel::text() const
{
    return m_inputText;
}

void AutoLabel::paintEvent(QPaintEvent *event)
{
    this->setText(m_inputText);
    return QLabel::paintEvent(event);
}

void AutoLabel::setInputText(const QString &text)
{
    m_inputText = text;
}
