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

#ifndef SYSTEMDATEFRAME_H
#define SYSTEMDATEFRAME_H

#include <QWidget>
#include <QScopedPointer>
#include <QLineEdit>

namespace installer {
class SystemDateFramePrivate;
class SystemDateFrame : public QWidget
{
    Q_OBJECT
public:
    explicit SystemDateFrame(QWidget *parent = nullptr);
    ~SystemDateFrame() override;

    QString timedate() const;
    bool isEnabled() const;

    bool validateTimeDate() const;
    void timeDateSetFinished();

signals:
    void finished();
    void cancel();

protected:
    bool event(QEvent* event) override;

private:
    QScopedPointer<SystemDateFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, SystemDateFrame)
};

}

#endif // SYSTEMDATEFRAME_H
