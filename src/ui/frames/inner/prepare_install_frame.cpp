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

#include "ui/frames/inner/prepare_install_frame.h"

#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <DFrame>
#include <DPalette>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>

#include "base/file_util.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/select_button.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "base/command.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace {
    const int kButtonWidth = 200;
    const int kButtonHeight = 36;

    const int kDescriptionWidth = 538;
    const int kDescriptionHeight = 308;
}

namespace installer {

PrepareInstallFrame::PrepareInstallFrame(QWidget* parent)
    : QFrame(parent) {
  this->setObjectName("prepare_install_frame");

  this->initUI();
  this->initConnections();
  // 无人值守只能是全盘，故不用考虑手动分区的情况
  WriteIsInitRecovery(GetSettingsBool(kIsInitRecvoery));
}

void PrepareInstallFrame::updateDescription(const QStringList& descriptions) {
  const QString prefix("•   ");
  QStringList modified_desc_list;
  QFontMetrics metrics(description_edit_->font());
  for (const QString& description : descriptions) {
    const QString content = prefix + description;
    modified_desc_list.append(content);
  }
  const QString description_text = modified_desc_list.join("\n");
  qDebug() << "description:" << description_text;
  description_edit_->setText(description_text);
}


void PrepareInstallFrame::setCreateRecovery(bool isCreate)
{
    if (!isCreate) {
        m_selectCreateRecovery->hide();
    } else {
        m_selectCreateRecovery->show();
    }
}

bool PrepareInstallFrame::focusSwitch()
{
    QScrollArea* testscroll = findChild<QScrollArea*>("scrollarea");
    if (continue_button_->hasFocus()) {
        abort_button_->setFocus();
    } else if (abort_button_->hasFocus()) {
        if (testscroll != nullptr) {
            testscroll->setFocus();
            m_scrollareaStyleSheetold = testscroll->styleSheet();
            testscroll->setStyleSheet("QWidget#scrollarea{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");
        } else {
            continue_button_->setFocus();
        }
    } else if (testscroll->hasFocus()) {
        testscroll->setStyleSheet(m_scrollareaStyleSheetold);
        if (m_selectCreateRecovery->isVisible()) {
            m_selectCreateRecovery->setFocus();
        } else {
            continue_button_->setFocus();
        }
    } else if (m_selectCreateRecovery->hasFocus()) {
        continue_button_->setFocus();
    } else {
        continue_button_->setFocus();
    }

    return true;
}

bool PrepareInstallFrame::doSpace()
{
    if (m_selectCreateRecovery->hasFocus()) {
        m_selectCreateRecovery->setChecked(!m_selectCreateRecovery->isChecked());
    }
    return true;
}

bool PrepareInstallFrame::doSelect()
{
    if (continue_button_->hasFocus()) {
        emit continue_button_->clicked();
    } else if (abort_button_->hasFocus()) {
        emit abort_button_->clicked();
    }
    return true;
}

bool PrepareInstallFrame::directionKey(int keyvalue)
{
    QScrollArea* testscroll = findChild<QScrollArea*>("scrollarea");
    switch (keyvalue) {
    case Qt::Key_Up: {
             if (testscroll != nullptr) {
                 if(testscroll->hasFocus()){
                     int testvalue = testscroll->verticalScrollBar()->value();
                     if ((testvalue - 20) >= 0) {
                         testscroll->verticalScrollBar()->setValue(testvalue - 20);
                     }
                 }
            }
        }
        break;
    case Qt::Key_Down: {
            if (testscroll != nullptr) {
                if(testscroll->hasFocus()){
                    int testvalue = testscroll->verticalScrollBar()->value();
                    if ((testvalue + 20) <= testscroll->verticalScrollBar()->maximum()) {
                        testscroll->verticalScrollBar()->setValue(testvalue + 20);
                    }
                }
            }
        }
        break;
    case Qt::Key_Left: {

        }
        break;
    case Qt::Key_Right: {

        }
        break;
    }

    return true;
}

void PrepareInstallFrame::installNvidiaStateChanged(bool install_nvidia) {
    WriteEnableNvidiaDriver(install_nvidia);
}

void PrepareInstallFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    updateTs();
  } else {
    QFrame::changeEvent(event);
  }
}

void PrepareInstallFrame::initConnections() {
    connect(abort_button_, &QPushButton::clicked,
            this, &PrepareInstallFrame::aborted);
    connect(continue_button_, &QPushButton::clicked, this, [=] {
        WriteIsInitRecovery(!m_selectCreateRecovery->isHidden()
                            && m_selectCreateRecovery->isChecked());
        emit finished();
    });
    connect(m_installNvidiaCheck, &QCheckBox::clicked, this, &PrepareInstallFrame::installNvidiaStateChanged);
}

void PrepareInstallFrame::initUI() {
  title_label_ = new TitleLabel(::QObject::tr("Ready to Install"));
  comment_label_ = new CommentLabel(
      ::QObject::tr("Make a backup of your important data and then continue"));

  description_edit_ = new QLabel();
  description_edit_->setWordWrap(true);
  description_edit_->adjustSize();
  description_edit_->setFixedWidth(kDescriptionWidth - 20);
  description_edit_->setFrameShape(QFrame::Shape::NoFrame);

  QPalette palette;
  palette.setColor(QPalette::Text, QColor(66, 154, 216));
  description_edit_->setPalette(palette);
  description_edit_->setForegroundRole(QPalette::Text);

  description_edit_->setObjectName("description_edit");
  description_edit_->setContentsMargins(0, 0, 0, 0);
  description_edit_->setContextMenuPolicy(Qt::NoContextMenu);
  description_edit_->setFrameStyle(QFrame::NoFrame);


  DFrame *sourceWidget = new DFrame(this);
  sourceWidget->setBackgroundRole(DPalette::ItemBackground);
  sourceWidget->setLineWidth(0);
  sourceWidget->setContentsMargins(0, 0, 0, 0);

  QHBoxLayout *sourceLayout = new QHBoxLayout;
  sourceLayout->setMargin(0);
  sourceLayout->setSpacing(0);
  sourceLayout->addWidget(description_edit_, 0, Qt::AlignTop | Qt::AlignHCenter);
  sourceLayout->addStretch();
  sourceLayout->setContentsMargins(0, 0, 0, 0);
  sourceWidget->setLayout(sourceLayout);

  QScrollArea* scroll = new QScrollArea(this);
  scroll->setFixedSize(kDescriptionWidth, kDescriptionHeight);
  scroll->setWidget(sourceWidget);
  scroll->setObjectName("scrollarea");
  scroll->setWidgetResizable(true);
  //scroll->setFocusPolicy(Qt::TabFocus);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  scroll->setContentsMargins(0, 0, 0, 0);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll->setContextMenuPolicy(Qt::NoContextMenu);
  scroll->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  scroll->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

  QScroller::grabGesture(scroll, QScroller::TouchGesture);
  scroll->setFixedWidth(540);
  QPalette pl3 = scroll->palette();
  pl3.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
  scroll->setPalette(pl3);

//  QHBoxLayout* descriptionLayout = new QHBoxLayout();
//  descriptionLayout->setContentsMargins(5, 5, 0, 5);
//  descriptionLayout->setSpacing(0);
//  descriptionLayout->addWidget(scroll);

  m_selectCreateRecovery = new DCheckBox;
  m_selectCreateRecovery->setChecked(GetSettingsBool(kIsInitRecvoery));
  m_selectCreateRecovery->setVisible(GetSettingsBool(kIsShowInitRecoveryCheckbox));


  m_installNvidiaCheck = new QCheckBox;
  m_installNvidiaCheck->setObjectName("check_box");
  m_installNvidiaCheck->setCheckable(true);
  m_installNvidiaCheck->setChecked(false);
  //m_installNvidiaCheck->setFocusPolicy(Qt::TabFocus);
  m_installNvidiaCheck->setText(::QObject::tr("Install NVIDIA closed source driver"));

  abort_button_ = new SelectButton();
  abort_button_->setFixedSize(kButtonWidth, kButtonHeight);
  //abort_button_->setFocusPolicy(Qt::TabFocus);
  continue_button_ = new DSuggestButton();
  continue_button_->setFixedSize(kButtonWidth, kButtonHeight);
  //continue_button_->setFocusPolicy(Qt::TabFocus);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(0);
  buttonLayout->addWidget(abort_button_, 0, Qt::AlignHCenter | Qt::AlignLeft);
  buttonLayout->addSpacing(20);
  buttonLayout->addWidget(continue_button_, 0, Qt::AlignHCenter | Qt::AlignRight);
  QWidget *buttonWrapWidget = new QWidget;
  buttonWrapWidget->setLayout(buttonLayout);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(title_label_, 0, Qt::AlignHCenter);
  layout->addWidget(comment_label_, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addWidget(scroll, 0, Qt::AlignHCenter);
  layout->addStretch();
  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->addStretch();
  h_layout->addWidget(m_selectCreateRecovery, 0, Qt::AlignHCenter);
  h_layout->addWidget(m_installNvidiaCheck, 0, Qt::AlignHCenter);
  h_layout->addStretch();
  h_layout->setSpacing(91);
  layout->addLayout(h_layout);
  layout->addSpacing(15);
  layout->addWidget(buttonWrapWidget, 0, Qt::AlignHCenter);

  setLayout(layout);
  updateTs();

  m_installNvidiaCheck->setVisible(scanNvidia());
}

void PrepareInstallFrame::updateTs()
{
    title_label_->setText(::QObject::tr("Ready to Install"));
    comment_label_->setText(
        ::QObject::tr("Make a backup of your important data and then continue"));
    abort_button_->setText(::QObject::tr("Back", "button"));
    continue_button_->setText(::QObject::tr("Continue"));
    m_selectCreateRecovery->setText(::QObject::tr("Create a backup for system restore, but it will increase the time"));
    m_installNvidiaCheck->setText(::QObject::tr("Install NVIDIA closed source driver"));
}

bool PrepareInstallFrame::scanNvidia()
{
    if (!GetSettingsBool(KEnableInstallNvidiaDriver) || isNotebook()) {
        return false;
    }

    QString cmd("lspci");
    QStringList args("-n");
    QString output;
    if (!SpawnCmd(cmd, args, output)) {
       return false;
    }

    return (output.indexOf(QRegExp(".*03(80|0[0-2]): 10de")) > -1);
}

}  // namespace installer
