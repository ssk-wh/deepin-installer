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

#include "operator_widget.h"

#include "base/file_util.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QCheckBox>

namespace  {
    const int kMargin = 5;
}

installer::OperatorWidget::OperatorWidget(QWidget *parent):
    DButtonBoxButton(QString(), parent),
    m_isCancelSelect(false)
{
    initUi();
    initConnection();
}

void installer::OperatorWidget::setTitle(const QString &text)
{
    m_titleLabel->setText(text);
}

void installer::OperatorWidget::setBody(const QString &text)
{
    m_bodyLabel->setText(text);
}

void installer::OperatorWidget::setSelectIcon(const QString &icon)
{
    const QPixmap defaultPixmap(icon);
    m_selectIconLabel->setPixmap(defaultPixmap);
    m_selectIconLabel->setFixedSize(defaultPixmap.size() / devicePixelRatioF());
}

void installer::OperatorWidget::setSelectMode(bool isCancel)
{
    m_isCancelSelect = isCancel;
}

void installer::OperatorWidget::setSelect(bool checked)
{
    m_selectIconLabel->setVisible(checked);
    setCheckable(checked);
}

void installer::OperatorWidget::selectChange()
{
    setFocus();
    if (m_isCancelSelect) {
        setCheckable(!isChecked());
        m_selectIconLabel->setVisible(!m_selectIconLabel->isVisible());
    } else {
        setCheckable(true);
        m_selectIconLabel->setVisible(true);
    }
}

void installer::OperatorWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath PaintPath;
    QColor color(0, 0, 0, 15);

    PaintPath.addRoundedRect(rect(), 10, 10);
    painter.fillPath(PaintPath, color);

    QWidget::paintEvent(event);
}

void installer::OperatorWidget::resizeEvent(QResizeEvent *event)
{
    m_titleLabel->setFixedWidth(width() - 2 * kMargin);
    m_bodyLabel->setFixedWidth(width() - 2 * kMargin);

    return DButtonBoxButton::resizeEvent(event);
}

void installer::OperatorWidget::initUi()
{
    setContentsMargins(kMargin, kMargin, kMargin, kMargin);

    QFont font;
    font.setWeight(QFont::DemiBold);
    m_titleLabel= new QLabel;
    m_titleLabel->setFont(font);
    m_titleLabel->adjustSize();
    m_titleLabel->setWordWrap(true);

    m_bodyLabel = new QLabel;
    m_bodyLabel->adjustSize();
    m_bodyLabel->setWordWrap(true);

    const QPixmap defaultPixmap(":/images/select.svg");
    m_selectIconLabel = new QLabel;
    m_selectIconLabel->setPixmap(defaultPixmap);
    m_selectIconLabel->setFixedSize(defaultPixmap.size() / devicePixelRatioF());
    m_selectIconLabel->hide();

    QVBoxLayout* textLayout = new QVBoxLayout;
    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_bodyLabel);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(textLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_selectIconLabel);
    setLayout(mainLayout);
}

void installer::OperatorWidget::initConnection()
{
    connect(this, &OperatorWidget::clicked, this, &OperatorWidget::selectChange);
}
