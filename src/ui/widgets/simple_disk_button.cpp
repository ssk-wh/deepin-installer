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

#include "ui/widgets/simple_disk_button.h"

#include <QLabel>
#include <QVBoxLayout>

#include "base/file_util.h"
#include "ui/delegates/partition_util.h"
#include "ui/utils/widget_util.h"
#include "service/screen_adaptation_manager.h"
#include "service/settings_name.h"
#include "service/settings_manager.h"

namespace installer {

namespace {

const int kButtonWidth = 260;
const int kButtonHeight = 220;

const QString kDriverIcon = GetPartitionIcon128();
const QString kDriverInstallIcon = GetPartitionIcon128();

}  // namespace

SimpleDiskButton::SimpleDiskButton(const Device::Ptr device, QWidget* parent)
  : QPushButton(parent),
    device_(device),
    selected_(false) {
  this->setObjectName("simple_disk_button");
  this->setCheckable(true);
  this->initUI();

  m_labelStyleSheetBackup = path_label->styleSheet();
}

void SimpleDiskButton::setSelected(bool selected) {
    selected_ = selected;
    os_label_->setPixmap(ScreenAdaptationManager::instance()->adapterPixmap(selected ? kDriverInstallIcon : kDriverIcon));

    if (selected) {
        path_label->setStyleSheet("QLabel {color: white;}");
        model_label->setStyleSheet("QLabel {color: white;}");
        size_label->setStyleSheet("QLabel {color: white;}");
    }
    else {
        path_label->setStyleSheet(m_labelStyleSheetBackup);
        model_label->setStyleSheet(m_labelStyleSheetBackup);
        size_label->setStyleSheet(m_labelStyleSheetBackup);
    }
}

void SimpleDiskButton::setupToolTip()
{
    this->setToolTip(QString(" path: %1\n mode: %2\n size: %3").arg(device_->path,
                                                 device_->model,
                                                 QString("%1 GB").arg(ToGigByte(device_->getByteLength()))));
}

void SimpleDiskButton::initUI() {
    // 设置的字体大小是跟随像素的
    QFont font;
    font.setPixelSize(installer::GetSettingsInt(installer::kSystemDefaultFontSize) - 2);

    os_label_ = new QLabel;
    os_label_->setObjectName("fs_label");
    os_label_->setPixmap(ScreenAdaptationManager::instance()->adapterPixmap(kDriverIcon));

    path_label = new QLabel;
    path_label->setObjectName("path_label");
    path_label->setText(device_->path);
    path_label->setFont(font);

    model_label = new QLabel;
    model_label->setObjectName("model_label");
    model_label->setText(device_->model);
    model_label->setFont(font);

    size_label = new QLabel();
    size_label->setObjectName("size_label");
    size_label->setText(QString("%1 GB").arg(ToGigByte(device_->getByteLength())));
    size_label->setFont(font);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(os_label_, 0, Qt::AlignHCenter);
    layout->addSpacing(1);
    layout->addWidget(path_label, 0, Qt::AlignHCenter);
    layout->addSpacing(1);
    layout->addWidget(model_label, 0, Qt::AlignHCenter);
    layout->addSpacing(1);
    layout->addWidget(size_label, 0, Qt::AlignHCenter);
    layout->addStretch();

    this->setLayout(layout);

    this->setCheckable(true);
    //this->setFixedSize(kButtonWidth, kButtonHeight);
    this->setMaximumSize(kButtonWidth, kButtonHeight);
    this->setMinimumSize(kButtonWidth - 40, kButtonHeight - 100);
    this->setupToolTip();
}

}  // namespace installer
