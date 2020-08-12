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

#ifndef INSTALLER_UI_FRAMES_SELECT_LANGUAGE_FRAME_H
#define INSTALLER_UI_FRAMES_SELECT_LANGUAGE_FRAME_H

#include "service/system_language.h"

#include <QFrame>
#include <QModelIndex>
#include <QScopedPointer>

class QTranslator;
class QCheckBox;
class QLabel;

namespace installer {

class LanguageListModel;
class UserAgreementDelegate;
class SelectLanguageFramePrivate;
// Select system language and update ui language on-the-fly.
class SelectLanguageFrame : public QFrame {
  Q_OBJECT

 public:
  explicit SelectLanguageFrame(UserAgreementDelegate * delegate, QWidget* parent = nullptr);
  ~SelectLanguageFrame();

  // Read default locale.
  void readConf();

  // Write locale to settings file.
  void writeConf();

  bool isChecked();

 signals:
  // Emitted when new language item is selected.
  void timezoneUpdated(const QString& timezone);

  // show UserLicense
  void requestShowUserLicense() const;

  // show oem UserLicense
  void requestShowOemUserLicense() const;

  void requestNextButtonEnable(bool enable);

  void requestApplyLanguage();

  void requestShowUserExperience();

  void requestPrivacyLicense();

 protected:
  // Update text of next_button_
  void changeEvent(QEvent* event) override;

  // Handles key press event of language_view_.
  bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<SelectLanguageFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private.get(), SelectLanguageFrame);
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_SELECT_LANGUAGE_FRAME_H
