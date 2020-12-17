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

#include "ui/frames/control_panel_frame.h"

#include <QDebug>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QStackedWidget>
#include <QTabBar>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include "base/file_util.h"
#include "service/log_manager.h"
#include "service/process_util.h"
#include "ui/widgets/pointer_button.h"
#include "ui/widgets/table_combo_box.h"
#include "qtermwidget5/qtermwidget.h"
#include "service/screen_adaptation_manager.h"
#include "service/settings_manager.h"

namespace installer {

namespace {

// Read log file each 1000ms.
const int kReadLogInterval = 1000;

const int kWindowWidth = 860;
const int kWindowHeight = 480;

const int kBtnHeight = 40;

const int kSettingsPageId = 2;

// Absolute path to settings file.
// Same in service/settings_manager.cpp.

#ifdef QT_DEBUG
const char kInstallerConfigFile[] = "/tmp/deepin-installer.conf";
#else
const char kInstallerConfigFile[] = "/etc/deepin-installer.conf";
#endif // QT_DEBUG

}  // namespace

ControlPanelFrame::ControlPanelFrame(QWidget* parent)
    : QFrame(parent){
  this->setObjectName("control_panel_frame");

  this->initUI();
  this->initConnections();
  this->readLog();
}

void ControlPanelFrame::toggleVisible() {
#ifndef QT_DEBUG
    if (!GetSettingsBool("system_debug")) {
        return;
    }
#endif // QT_DEBUG
    this->setVisible(!this->isVisible());
    this->raise();
}

void ControlPanelFrame::initConnections() {
  connect(page_combo_box_,
          static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &ControlPanelFrame::currentPageChanged);
  connect(tab_bar_, &QTabBar::currentChanged,
          this, &ControlPanelFrame::onTabBarChanged);
  connect(log_viewer_, &QTextEdit::cursorPositionChanged,
          this, &ControlPanelFrame::onLogViewerCursorPositionChanged);

  connect(refresh_devices_button_, &QPushButton::clicked,
          this, &ControlPanelFrame::requestRefreshDevices);
  connect(refresh_qr_button_, &QPushButton::clicked,
          this, &ControlPanelFrame::requestRefreshQR);
  connect(simulate_slide_button_, &QPushButton::clicked,
          this, &ControlPanelFrame::requestSimulateSlide);
  connect(suicide_button_, &QPushButton::clicked,
          this, &ControlPanelFrame::onSuicideButtonClicked);

  connect(ScreenAdaptationManager::instance(),
          &ScreenAdaptationManager::primaryAvailableGetometryChanaged,
          this, [=](const QRect &rect) {
      this->setFixedSize(QSize(rect.width() / 2, rect.height() / 2));
      this->adjustSize();
  });
}

void ControlPanelFrame::initUI() {
  tab_bar_ = new QTabBar();
  tab_bar_->setShape(QTabBar::RoundedEast);
  tab_bar_->addTab("Log");
  tab_bar_->addTab("Pages");
  tab_bar_->addTab("Settings");
  tab_bar_->addTab("Terminal");

  log_viewer_ = new QTextEdit();
  log_viewer_->setObjectName("log_viewer");
  // Disable user modification.
  log_viewer_->setReadOnly(true);
  // Disable rich text support.
  log_viewer_->setAcceptRichText(false);
  // Disable context menu.
  log_viewer_->setContextMenuPolicy(Qt::NoContextMenu);
  log_viewer_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  log_viewer_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  page_combo_box_ = new TableComboBox();
  page_combo_box_->addItems({"ConfirmQuitFrame",
                             "DiskSpaceInsufficientFrame",
                             "InstallProgressFrame",
                             "PartitionFrame",
                             "PrivilegeErrorFrame",
                             "SelectLanguageFrame",
                             "SystemInfoFrame",
                             "VirtualMachineFrame",
                             "InstallResults",
                            });

  refresh_devices_button_ = new PointerButton();
  refresh_devices_button_->setObjectName("refresh_devices");
  refresh_devices_button_->setText("Refresh Device List");
  refresh_devices_button_->setFixedHeight(kBtnHeight);

  refresh_qr_button_ = new PointerButton();
  refresh_qr_button_->setObjectName("refresh_qr");
  refresh_qr_button_->setText("Refresh QR");
  refresh_qr_button_->setFixedHeight(kBtnHeight);

  simulate_slide_button_ = new PointerButton();
  simulate_slide_button_->setObjectName("simulate_slide");
  simulate_slide_button_->setText("Start slide");
  simulate_slide_button_->setFixedHeight(kBtnHeight);

  suicide_button_ = new PointerButton();
  suicide_button_->setObjectName("suicide_button");
  suicide_button_->setText("Suicide");
  suicide_button_->setFixedHeight(kBtnHeight);

  QHBoxLayout* page_layout = new QHBoxLayout();
  page_layout->addWidget(page_combo_box_, 0,
                         Qt::AlignLeft | Qt::AlignTop);
  page_layout->addWidget(refresh_devices_button_, 0,
                         Qt::AlignLeft | Qt::AlignTop);
  page_layout->addWidget(refresh_qr_button_, 0,
                         Qt::AlignLeft | Qt::AlignTop);
  page_layout->addWidget(simulate_slide_button_, 0,
                         Qt::AlignLeft | Qt::AlignTop);
  page_layout->addWidget(suicide_button_, 0,
                         Qt::AlignLeft | Qt::AlignTop);
  page_layout->addStretch();

  QFrame* page_frame = new QFrame();
  page_frame->setContentsMargins(0, 0, 0, 0);
  page_frame->setLayout(page_layout);

  settings_viewer_ = new QTextEdit();
  settings_viewer_->setObjectName("settings_viewer");
  // Disable user modification.
  settings_viewer_->setReadOnly(true);
  // Disable rich text support.
  settings_viewer_->setAcceptRichText(false);
  // Disable context menu.
  settings_viewer_->setContextMenuPolicy(Qt::NoContextMenu);
  settings_viewer_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  settings_viewer_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  term_widget_ = new QTermWidget;
  QFont term_font = this->font();
  term_font.setPixelSize(14);
  term_font.setFamily("Monospace");
  term_widget_->setTerminalFont(term_font);
  // Hide scroll bar.
  term_widget_->setScrollBarPosition(QTermWidget::NoScrollBar);
  term_widget_->setColorScheme("WhiteOnBlack");
  // Set bash as default shell program.
  term_widget_->setShellProgram("/bin/bash");

  stacked_widget_ = new QStackedWidget();
  stacked_widget_->addWidget(log_viewer_);
  stacked_widget_->addWidget(page_frame);
  stacked_widget_->addWidget(settings_viewer_);
  stacked_widget_->addWidget(term_widget_);

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(stacked_widget_);
  layout->addWidget(tab_bar_);

  this->setLayout(layout);
  this->setContentsMargins(0, 0, 0, 0);
  this->setStyleSheet(ReadFile(":/styles/control_panel_frame.css"));
  this->setFixedSize(kWindowWidth, kWindowHeight);
}

void ControlPanelFrame::onLogViewerCursorPositionChanged() {
  // Highlight current line.
  QTextEdit::ExtraSelection highlight;
  highlight.cursor = log_viewer_->textCursor();
  highlight.cursor.clearSelection();
  highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
  highlight.format.setBackground(QBrush(QColor(65, 65, 65)));
  log_viewer_->setExtraSelections({highlight});
}

void ControlPanelFrame::onTabBarChanged(int index) {
  if (index == kSettingsPageId) {
    const QString content = ReadFile(kInstallerConfigFile);
    settings_viewer_->setText(content);
  }
  stacked_widget_->setCurrentIndex(index);
}

void ControlPanelFrame::readLog() {
  const QString content(ReadFile(GetLogFilepath()));

  QTextCursor cursor = log_viewer_->textCursor();
  // Restore vertical position when log content is updated.
  const int pos = log_viewer_->verticalScrollBar()->value();
  log_viewer_->setPlainText(content);
  log_viewer_->verticalScrollBar()->setValue(pos);
}

void ControlPanelFrame::onSuicideButtonClicked() {
    Suicide();
}

void ControlPanelFrame::primaryGeometryChanged(const QRect &rect)
{

}

}  // namespace installer
