/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <zhangdingyuan@deepin.com>
 *
 * Maintainer: justforlxz <zhangdingyuan@deepin.com>
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

#ifndef LANGUAGEFRAME_H
#define LANGUAGEFRAME_H

#include "ui/interfaces/frameinterface.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include <QWidget>
#include <QStackedLayout>
#include <QScopedPointer>

namespace installer {
class SelectLanguageFrame;
class UserAgreementFrame;
class UserAgreementDelegate;
class LanguageFramePrivate;

class LanguageFrame : public FrameInterface {
    Q_OBJECT
    friend LanguageFramePrivate;
public:
    LanguageFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~LanguageFrame() override;

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;
    QString returnFrameName() const override;
    void acceptLicense(bool accept) const;

protected:
    void changeEvent(QEvent* event) override;

signals:
    // Emitted when new language item is selected.
    void timezoneUpdated(const QString& timezone);

    void coverMainWindowFrameLabelsView(bool cover);

private:
    QScopedPointer<LanguageFramePrivate> m_private;
    Q_DECLARE_PRIVATE_D(m_private, LanguageFrame)
};

}  // namespace installer

#endif  // !LANGUAGEFRAME_H
