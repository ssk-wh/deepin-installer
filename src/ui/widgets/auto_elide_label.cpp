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

#include "auto_elide_label.h"

namespace installer {

AutoElideLabel::AutoElideLabel(QWidget *parent)
    : QLabel(parent)
{
}

void AutoElideLabel::setFixedWidth(int w)
{
    QLabel::setFixedWidth(w);
    setText(text());
}

void AutoElideLabel::setFont(const QFont &font)
{
    QLabel::setFont(font);
    setText(text());
}

void AutoElideLabel::setText(const QString &text)
{
    QFontMetrics fontWidth(font());
    QString elideNote = fontWidth.elidedText(text, Qt::ElideRight, width());

    QLabel::setText(elideNote);
}

}

