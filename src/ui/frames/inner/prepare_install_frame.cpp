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
}

void PrepareInstallFrame::updateDescription(const QStringList& descriptions) {
  const QString prefix("•   ");
  QStringList modified_desc_list;
  QFontMetrics metrics(description_edit_->font());
  for (const QString& description : descriptions) {
    const QString content = prefix + description;
    modified_desc_list.append(content);
#ifdef QT_DEBUG
    modified_desc_list.append(content + "--TEST1");
    modified_desc_list.append(content + "--TEST2");
    modified_desc_list.append(content + "--TEST3");
    modified_desc_list.append(content + "--TEST4");
#endif //QT_DEBUG
  }
  const QString description_text = modified_desc_list.join("\n");
  qDebug() << "description:" << description_text;
  description_edit_->setText(description_text);
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
        WriteIfDoRecovery(m_selectCreateRecovery->isChecked());
        emit finished();
    });
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
  scroll->setFixedWidth(kDescriptionWidth);
  scroll->setWidget(sourceWidget);
  scroll->setObjectName("scrollarea");
  scroll->setWidgetResizable(true);
  scroll->setFocusPolicy(Qt::TabFocus);
  scroll->setFrameStyle(QFrame::NoFrame);
  scroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
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

  QHBoxLayout* descriptionLayout = new QHBoxLayout();
  descriptionLayout->setContentsMargins(5, 5, 0, 5);
  descriptionLayout->setSpacing(0);
  descriptionLayout->addWidget(scroll);

  m_selectCreateRecovery = new DCheckBox;
  m_selectCreateRecovery->setCheckable(true);
  m_selectCreateRecovery->setChecked(true);

  abort_button_ = new SelectButton();
  abort_button_->setFixedSize(kButtonWidth, kButtonHeight);
  abort_button_->setFocusPolicy(Qt::TabFocus);
  continue_button_ = new DSuggestButton();
  continue_button_->setFixedSize(kButtonWidth, kButtonHeight);
  continue_button_->setFocusPolicy(Qt::TabFocus);

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
  layout->addSpacing(30);
  layout->addWidget(title_label_, 0, Qt::AlignHCenter);
  layout->addWidget(comment_label_, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addWidget(scroll, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addWidget(m_selectCreateRecovery, 0, Qt::AlignHCenter);
  layout->addSpacing(15);
  layout->addWidget(buttonWrapWidget, 0, Qt::AlignHCenter);

  setLayout(layout);

  updateTs();
}

void PrepareInstallFrame::updateTs()
{
    title_label_->setText(::QObject::tr("Ready to Install"));
    comment_label_->setText(
        ::QObject::tr("Make a backup of your important data and then continue"));
    abort_button_->setText(::QObject::tr("Back"));
    continue_button_->setText(::QObject::tr("Continue"));
    m_selectCreateRecovery->setText(::QObject::tr("Create a backup for system restore, but it will increase the time"));
}

}  // namespace installer
