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

#ifndef INSTALLER_UI_WIDGETS_LINE_EDIT_H
#define INSTALLER_UI_WIDGETS_LINE_EDIT_H

#include <DIconButton>
#include <QLineEdit>

DWIDGET_USE_NAMESPACE

class QFocusEvent;
class QLabel;
class QResizeEvent;
class QValidator;

namespace installer {

// Customized line edit used in form page.
class LineEdit : public QFrame {
    Q_OBJECT

public:
    LineEdit(const QString& icon, QWidget* parent = nullptr);
    void setCapsLockVisible(bool visible);
    void setEchoMode(QLineEdit::EchoMode mode);

    QString text() const;
    void setText(const QString &text);

    void setPlaceholderText(const QString &text);

    bool isReadOnly() const;
    void setReadOnly(bool readOnly);

    void setValidator(const QValidator *validator);

    bool hasFocus() const;
    void setFocus();

    QLineEdit* lineEdit() const;

signals:
    void gotFocus();

    void editingFinished();
    void textEdited(const QString &text);
    void textChanged(const QString &text);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    void initConnection();
    void onFocusChanged(QWidget *old, QWidget *now);
    void resetLineEditMaxWidth(bool visible);

    QLabel* image_label_  = nullptr;
    QLineEdit* m_lineEdit = nullptr;
    QLabel* m_caseIndicateLabel = nullptr;
    DIconButton* m_switchPasswdModeBtn = nullptr;
};

}  // namespace installer

#endif  // INSTALLER_UI_WIDGETS_LINE_EDIT_H
