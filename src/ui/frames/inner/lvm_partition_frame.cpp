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

#include "ui/frames/inner/lvm_partition_frame.h"

#include <QPushButton>

#include "ui/interfaces/frameinterfaceprivate.h"

namespace installer {

LvmPartitionFrame::LvmPartitionFrame(
    AdvancedPartitionDelegate* delegate_, QWidget* parent)
    : AdvancedPartitionFrame (delegate_, parent) {
    bootloader_tip_button_->hide();
    bootloader_button_->hide();

    m_lastButton = new QPushButton;
    m_lastButton->setFixedSize(NEXTBTN_WIDTH, NEXTBTN_HEIGHT);
    m_lastButton->setFocusPolicy(Qt::TabFocus);

    connect(m_lastButton, &QPushButton::clicked, this, &LvmPartitionFrame::onLastButtonClicked);
}

void LvmPartitionFrame::updateLayout(QHBoxLayout* layout, QString text) {
    m_lastButtonLayout = layout;

    QRect rect = m_lastButtonLayout->geometry();
    int margin = rect.width() - 2 * m_lastButton->width() -10;
    m_lastButtonLayout->setContentsMargins(margin / 2, 0, margin / 2, 0);
    m_lastButtonLayout->insertWidget(0, m_lastButton);
    m_lastButton->setText(text);
    m_lastButton->show();

//    // Register Back button text
    LanguageManager::translator(m_lastButton, &QPushButton::setText, TranslatorType::BackButton);
}

void LvmPartitionFrame::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        m_lastButton->setText(::QObject::tr("Back"));
    }
    return AdvancedPartitionFrame::changeEvent(event);
}

void LvmPartitionFrame::onLastButtonClicked() {
    m_lastButton->hide();
    m_lastButtonLayout->removeWidget(m_lastButton);

    if (VgDevice::p_installer_VgDevice) {
        //由于智能指针会自动释放，此处不需要手动删除
        VgDevice::p_installer_VgDevice = nullptr;
    }

    this->clearErrorMessages();
    AdvancedPartitionDelegate::install_Lvm_Status = Install_Lvm_Status::Lvm_No_Need;
    emit aborted();
}

}  // namespace installer
