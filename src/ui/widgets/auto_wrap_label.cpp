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

#include "auto_wrap_label.h"

namespace installer {
AutoWrapLabel::AutoWrapLabel(QWidget *parent)
    : QLabel(parent)
{

}

void AutoWrapLabel::setText(const QString &string)
{
    m_inputText = string;

    if (string.isEmpty()) {
        return QLabel::setText(string);
    }

    // 计算一行显示的字符的size
    int perCharWidth = QLabel::fontMetrics().width(string.at(0));
    int perLineWidth = this->width() - 2 * this->margin();
    int perLineCount = perLineWidth / perCharWidth;

    QStringList setList;
    QStringList lineList = string.split("\n");

    // 对输入的每一行就行处理，如果长度超出每一行的size，则在超出的每部分上插入"\n"
    foreach (QString line, lineList) {
        if (line.size() <= perLineCount) {
            setList.append(line);
        }
        else {
            for (int pos = line.size() / perLineCount; pos > 0; pos--) {
                line.insert(pos * perLineCount, "\n");
            }

            setList.append(line);
        }
    }

    return QLabel::setText(setList.join("\n"));
}

QString AutoWrapLabel::text() const
{
    return m_inputText;
}

void AutoWrapLabel::paintEvent(QPaintEvent *event)
{
    this->setText(m_inputText);
    return QLabel::paintEvent(event);
}
}
