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

#include "ui/frames/inner/install_progress_slide_frame.h"

#include <QDebug>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QHBoxLayout>
#include <QTimer>
#include <QResizeEvent>

#include "service/settings_manager.h"
#include "ui/delegates/install_slide_frame_util.h"
#include "ui/utils/widget_util.h"

namespace {
    const int kAnimationWidth = 412;
    const int kAnimationHeight = 232;
}

namespace installer {

InstallProgressSlideFrame::InstallProgressSlideFrame(QWidget* parent)
    : QFrame(parent),
      slide_index_(0),
      slide_files_() {
  this->setObjectName("install_progress_frame");

  this->initUI();
  this->initConnections();
}

void InstallProgressSlideFrame::startSlide(bool disable_slide,
                                           bool disable_animation,
                                           int duration) {
  qDebug() << "startSlide()" << disable_slide << disable_animation << duration;
  const QString locale = ReadLocale();
  slide_files_ = GetSlideFiles(locale);

  if (slide_files_.isEmpty()) {
    qCritical() << "startSlide() no slide files found!";
    return;
  }

  // Read the first slide image.
  slide_index_ = 0;
  this->updateSlideImage();

  if (disable_slide) {
    qDebug() << "slide disabled";
    return;
  }

  animation_group_->clear();
  if (disable_animation) {
    qDebug() << "slide animation disabled";
    null_animation_->setDuration(duration);
    animation_group_->addAnimation(null_animation_);
  } else {
    pos_animation_->setDuration(duration);
    animation_group_->addAnimation(pos_animation_);
    opacity_animation_->setDuration(duration);
    animation_group_->addAnimation(opacity_animation_);
  }
  animation_group_->start();
}

void InstallProgressSlideFrame::stopSlide() {
  animation_group_->stop();
}

void InstallProgressSlideFrame::initConnections() {
  connect(animation_group_, &QParallelAnimationGroup::currentLoopChanged,
          this, &InstallProgressSlideFrame::onAnimationCurrentLoopChanged);
  connect(m_backButton, &DIconButton::clicked, this, &InstallProgressSlideFrame::onBackButtonClicked);
  connect(m_nextButton, &DIconButton::clicked, this, &InstallProgressSlideFrame::onNextButtonClicked);
}

void InstallProgressSlideFrame::initUI() {
  m_animationContainer = new QWidget;
  m_animationContainer->setMinimumSize(QSize(kAnimationWidth, kAnimationHeight));
  container_label_ = new QLabel(m_animationContainer);

  if (!GetSlideFiles(ReadLocale()).isEmpty()) {
     m_animationContainer->setFixedSize(QPixmap(GetSlideFiles(ReadLocale()).at(0)).size());
     container_label_->setFixedSize(QPixmap(GetSlideFiles(ReadLocale()).at(0)).size());
  }

  m_backButton = new DIconButton(this);
  m_backButton->setIcon(QIcon(":/images/backPicture.svg"));
  m_backButton->setIconSize(QSize(20, 20));
  m_backButton->setFocusPolicy(Qt::NoFocus);

  m_nextButton = new DIconButton(this);
  m_nextButton->setIcon(QIcon(":/images/nextPicture.svg"));
  m_nextButton->setIconSize(QSize(20, 20));
  m_nextButton->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addStretch();
  layout->addWidget(m_backButton, 0, Qt::AlignRight | Qt::AlignVCenter);
  layout->addSpacing(0);
  layout->addWidget(m_animationContainer, 0, Qt::AlignCenter);
  layout->addSpacing(0);
  layout->addWidget(m_nextButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
  layout->addStretch();

  setLayout(layout);

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  pos_animation_ = new QPropertyAnimation(container_label_, "pos", m_animationContainer);
  pos_animation_->setKeyValueAt(0.0, QPoint(-50, 0));
  pos_animation_->setKeyValueAt(0.1, QPoint(0, 0));
  pos_animation_->setKeyValueAt(0.9, QPoint(0, 0));
  pos_animation_->setKeyValueAt(1.0, QPoint(50, 0));

  opacity_effect_ = new QGraphicsOpacityEffect(container_label_);
  container_label_->setGraphicsEffect(opacity_effect_);
  opacity_animation_ = new QPropertyAnimation(opacity_effect_, "opacity", m_animationContainer);
  opacity_animation_->setKeyValueAt(0.0, 0.0);
  opacity_animation_->setKeyValueAt(0.1, 1.0);
  opacity_animation_->setKeyValueAt(0.9, 1.0);
  opacity_animation_->setKeyValueAt(1.0, 0.0);

  null_animation_ = new QVariantAnimation(this);

  animation_group_ = new QParallelAnimationGroup(this);
  animation_group_->setLoopCount(-1);
}

void InstallProgressSlideFrame::updateSlideImage() {
  Q_ASSERT(slide_index_ < slide_files_.length());
  const QString filepath(slide_files_.at(slide_index_));
  if (QFile::exists(filepath)) {
    m_cachePixmap = installer::renderPixmap(filepath);
  }
  else {
    qWarning() << "slide file not found:" << filepath;
  }

  updateSlidePixmap();
  container_label_->show();

  slide_index_ = (slide_index_ + 1) % slide_files_.length();
}

void InstallProgressSlideFrame::updateSlidePixmap()
{
    const int buttonWidth = m_backButton->width();
    constexpr int buttonSpacing = 20;
    const int width = this->width() - (buttonWidth + buttonSpacing) * 2;

    container_label_->setPixmap(m_cachePixmap);
}

void InstallProgressSlideFrame::showEvent(QShowEvent *event)
{
    m_backButton->setCheckable(false);
    m_nextButton->setCheckable(false);
    m_animationContainer->setFocus();
    return QFrame::showEvent(event);
}

void InstallProgressSlideFrame::onAnimationCurrentLoopChanged() {
    updateSlideImage();
}

void InstallProgressSlideFrame::onBackButtonClicked()
{
    slide_index_ = (slide_files_.length() + slide_index_ - 2) % slide_files_.length();
    updateSlideImage();
}

void InstallProgressSlideFrame::onNextButtonClicked()
{
    updateSlideImage();
}

}  // namespace installer
