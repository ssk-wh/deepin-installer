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

#ifndef AUTOLABEL_H
#define AUTOLABEL_H

#include <QLabel>

class AutoLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AutoLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    explicit AutoLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());

    QString text() const;

public Q_SLOTS:
    void setText(const QString &);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setInputText(const QString &text);

private:
    QString m_inputText;        // 保存setText中原始字符串
};

#endif // AUTOLABEL_H
