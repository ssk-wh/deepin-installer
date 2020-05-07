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

#include "base/file_util.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"

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
  }
  const QString description_text = modified_desc_list.join("\n");
  qDebug() << "description:" << description_text;
  description_edit_->setPlainText(description_text);
}

void PrepareInstallFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(tr("Ready to Install"));
    comment_label_->setText(
        tr("Make a backup of your important data and then continue"));
    abort_button_->setText(tr("Back"));
    continue_button_->setText(tr("Continue"));
  } else {
    QFrame::changeEvent(event);
  }
}

void PrepareInstallFrame::initConnections() {
  connect(abort_button_, &QPushButton::clicked,
          this, &PrepareInstallFrame::aborted);
  connect(continue_button_, &QPushButton::clicked,
          this, &PrepareInstallFrame::finished);
}

void PrepareInstallFrame::initUI() {
  title_label_ = new TitleLabel(tr("Ready to Install"));
  comment_label_ = new CommentLabel(
      tr("Make a backup of your important data and then continue"));

  description_edit_ = new QTextEdit();
  description_edit_->setFixedWidth(kDescriptionWidth - 20);
  description_edit_->setLineWrapMode(QTextEdit::WidgetWidth);

  description_edit_->setFrameShape(QFrame::Shape::NoFrame);
  QPalette pl = description_edit_->palette();
  pl.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
  description_edit_->setPalette(pl);

  description_edit_->setObjectName("description_edit");
  description_edit_->setContentsMargins(0, 0, 0, 0);
  description_edit_->setAcceptRichText(false);
  description_edit_->setReadOnly(true);
  description_edit_->setContextMenuPolicy(Qt::NoContextMenu);
  description_edit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  description_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QHBoxLayout* descriptionLayout = new QHBoxLayout();
  descriptionLayout->setContentsMargins(10, 10, 10, 10);
  descriptionLayout->setSpacing(0);
  descriptionLayout->addWidget(description_edit_);

  DFrame *m_bgGroup = new DFrame(this);
  m_bgGroup->setBackgroundRole(DPalette::ItemBackground);
  m_bgGroup->setLineWidth(0);
  m_bgGroup->setContentsMargins(0, 0, 0, 0);
  m_bgGroup->setLayout(descriptionLayout);
  m_bgGroup->setFixedSize(QSize(kDescriptionWidth, kDescriptionHeight));

  abort_button_ = new QPushButton(tr("Back"));
  abort_button_->setFixedSize(kButtonWidth, kButtonHeight);
  continue_button_ = new QPushButton(tr("Continue"));
  continue_button_->setFixedSize(kButtonWidth, kButtonHeight);

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
  layout->addWidget(m_bgGroup, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addWidget(buttonWrapWidget, 0, Qt::AlignHCenter);

  setLayout(layout);
}

}  // namespace installer
