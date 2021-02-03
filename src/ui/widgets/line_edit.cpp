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

#include "ui/widgets/line_edit.h"
#include "base/file_util.h"
#include "ui/utils/widget_util.h"

#include <QFocusEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QValidator>
#include <QPainter>
#include <QApplication>
#include <QPainterPath>

namespace installer {

LineEdit::LineEdit(const QString& icon, QWidget* parent)
    : QFrame(parent)
{
    this->setObjectName("line_edit");

    this->setStyleSheet(ReadFile(":/styles/line_edit.css"));
    // Same width as in table combobox.
    this->setFixedSize(340, 36);

    // Disable context menu.
    this->setContextMenuPolicy(Qt::NoContextMenu);

    image_label_ = new QLabel(this);
    image_label_->setPixmap(installer::renderPixmap(icon));
    image_label_->setFixedSize(12, 12);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setMinimumWidth(249);
    m_lineEdit->setFixedHeight(36);
    m_lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_lineEdit->installEventFilter(this);

    m_caseIndicateLabel = new QLabel(this);
    m_caseIndicateLabel->setFixedSize(12, 12);
    m_caseIndicateLabel->setPixmap(installer::renderPixmap(":/images/capslock.svg"));
    m_caseIndicateLabel->hide();

    m_switchPasswdModeBtn = new DIconButton(this);
    m_switchPasswdModeBtn->setFixedSize(12, 12);
    m_switchPasswdModeBtn->setIconSize(QSize(12, 12));
    m_switchPasswdModeBtn->setIcon(QIcon(":/images/password_show_normal.svg"));
    m_switchPasswdModeBtn->setFlat(true);
    m_switchPasswdModeBtn->hide();
    m_switchPasswdModeBtn->installEventFilter(this);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(5);
    mainLayout->addWidget(image_label_);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_lineEdit, 0, Qt::AlignLeft);
    mainLayout->addWidget(m_caseIndicateLabel, 0, Qt::AlignRight);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_switchPasswdModeBtn);
    mainLayout->addSpacing(5);

    setObjectName("LineEdit");
    // Same width as in table combobox.
    setFixedSize(310, 36);
    setLayout(mainLayout);
    setStyleSheet(ReadFile(":/styles/line_edit.css"));

    initConnection();
}

void LineEdit::setCapsLockVisible(bool visible)
{
    m_caseIndicateLabel->setVisible(visible);
    resetLineEditMaxWidth(visible);
}

void LineEdit::setEchoMode(QLineEdit::EchoMode mode)
{
    m_lineEdit->setEchoMode(mode);
    m_switchPasswdModeBtn->setVisible(mode == QLineEdit::Password);
}

QString LineEdit::text() const
{
    return m_lineEdit->text();
}

void LineEdit::setText(const QString &text)
{
    m_lineEdit->setText(text);
}

void LineEdit::setPlaceholderText(const QString &text)
{
    m_lineEdit->setPlaceholderText(text);
}

bool LineEdit::isReadOnly() const
{
    return m_lineEdit->isReadOnly();
}

void LineEdit::setReadOnly(bool readOnly)
{
    m_lineEdit->setReadOnly(readOnly);
}

void LineEdit::setValidator(const QValidator *validator)
{
    m_lineEdit->setValidator(validator);
}

bool LineEdit::hasFocus() const
{
    return m_lineEdit->hasFocus();
}

void LineEdit::setFocus()
{
    m_lineEdit->setFocus();
}

QLineEdit *LineEdit::lineEdit() const
{
    return m_lineEdit;
}

void LineEdit::resetLineEditMaxWidth(bool visible)
{
    if (visible){
        m_lineEdit->setFixedWidth(249);
    }
    else {
        m_lineEdit->setFixedWidth(261);
    }
}

bool LineEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_switchPasswdModeBtn){
        if (event->type() == QMouseEvent::MouseButtonPress){
            if (m_lineEdit->echoMode() == QLineEdit::Normal){
                m_lineEdit->setEchoMode(QLineEdit::Password);

                m_switchPasswdModeBtn->setIcon(QIcon(":/images/password_show_normal.svg"));
            }
            else {
                m_lineEdit->setEchoMode(QLineEdit::Normal);

                m_switchPasswdModeBtn->setIcon(QIcon(":/images/password_hide_normal.svg"));
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void LineEdit::paintEvent(QPaintEvent *)
{
    if (m_lineEdit->hasFocus()){
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPainterPath path;
        path.addRoundedRect(rect(), 4, 4);
        painter.setClipPath(path);

        painter.setPen(QPen(QColor("#2ca7f8"), 1, Qt::SolidLine, Qt::RoundCap));

        painter.drawRect(rect());
        painter.end();
    }
}

void LineEdit::initConnection()
{
    connect(m_lineEdit, &QLineEdit::editingFinished, this, &LineEdit::editingFinished);
    connect(m_lineEdit, &QLineEdit::textEdited, this, &LineEdit::textEdited);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &LineEdit::textChanged);

    connect(qApp, &QApplication::focusChanged, this, &LineEdit::onFocusChanged);
}

void LineEdit::onFocusChanged(QWidget *old, QWidget *now)
{
    if (old != m_lineEdit && now != m_lineEdit){
        return;
    }

    if (old == m_lineEdit) {
        emit editingFinished();
    }

    if (now == m_lineEdit){
        emit gotFocus();
    }

    update();
}

}  // namespace installer
