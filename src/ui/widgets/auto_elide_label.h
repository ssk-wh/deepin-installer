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

#pragma once

#include <QLabel>

namespace installer {

class AutoElideLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AutoElideLabel(QWidget *parent = nullptr);

    void setFixedWidth(int w);
    void setFont(const QFont& font);

    QString text() const;
    void setText(const QString& text);

private:
    QString m_originalText;
};

}

