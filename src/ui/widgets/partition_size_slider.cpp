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

#include "ui/widgets/partition_size_slider.h"

#include <QHBoxLayout>
#include <QRegExpValidator>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPainter>
#include <QPainterPath>

#include "base/file_util.h"
#include "partman/structs.h"

namespace installer {

PartitionSizeSlider::PartitionSizeSlider(QWidget* parent)
    : QFrame(parent),
      maximum_size_(0),
      minimum_size_(0) {
  this->setObjectName("partition_size_slider");

  this->initUI();
  this->initConnection();
}

qint64 PartitionSizeSlider::value() {
  // Keep maximum value unchanged.
  if (slider_->value() == slider_->maximum()) {
    return maximum_size_;
  } else {
    return slider_->value() * kMebiByte;
  }
}

void PartitionSizeSlider::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath PaintPath;
    for (int i = 0; i <10; i++)
    {
        const QColor fullAreaColor(0, 0, 0, 6);

        PaintPath.addRoundedRect(rect(), 11, 11);
        painter.fillPath(PaintPath, fullAreaColor);
    }
}

void PartitionSizeSlider::setMaximum(qint64 maximum_size) {
  Q_ASSERT(maximum_size >= 0);
  maximum_size_ = maximum_size;
  // First convert bytes to mebibytes.
  const int mebi_size = static_cast<int>(maximum_size / kMebiByte);
  slider_->setMaximum(mebi_size);
  slider_->setValue(mebi_size);
}

void PartitionSizeSlider::setMinimum(qint64 minimum_size) {
  Q_ASSERT(minimum_size >= 0);
  // Make sure that |minimum_size| is in range.
  const qint64 min_size = qMin(minimum_size, maximum_size_);
  minimum_size_ = min_size;
  // Convert bytes to mebibytes.
  const int mebi_size = static_cast<int>(min_size / kMebiByte);
  slider_->setMinimum(mebi_size);
}

void PartitionSizeSlider::setValue(qint64 size) {
  Q_ASSERT(size >= minimum_size_);
  Q_ASSERT(size <= maximum_size_);
  qint64 real_size = qMin(size, maximum_size_);
  real_size = qMax(size, minimum_size_);
  // Convert to mebibytes.
  const int mebi_size = static_cast<int>(real_size / kMebiByte);
  slider_->setValue(mebi_size);
  editor_->setText(QString("%1").arg(mebi_size));
}

void PartitionSizeSlider::initConnection() {
  connect(slider_, &QSlider::valueChanged,
          this, &PartitionSizeSlider::onSliderValueChanged);
  connect(editor_, &QLineEdit::textChanged,
          this, &PartitionSizeSlider::onEditorTextChanged);
  connect(editor_, &QLineEdit::editingFinished,
          this, &PartitionSizeSlider::onEditorFinished);
}

void PartitionSizeSlider::initUI() {
  slider_ = new QSlider(Qt::Horizontal);
  slider_->setObjectName("slider");
  slider_->setMinimum(0);

  editor_ = new QLineEdit();
  editor_->setObjectName("editor");
  editor_->setValidator(new QRegExpValidator(QRegExp("[0-9]*")));
  editor_->setFixedWidth(90);
  // Disable context menu.
  editor_->setContextMenuPolicy(Qt::NoContextMenu);
  //editor_->setFocusPolicy(Qt::TabFocus);
  connect(editor_, &QLineEdit::selectionChanged, this,[=] {
    editor_->setFocus();
  });

  connect(editor_, &QLineEdit::editingFinished, this,[=] {
    this->setFocus();
  });

  QLabel* size_label = new QLabel("MB");
  size_label->setObjectName("size_label");

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setContentsMargins(0, 2, 0, 2);
  layout->setSpacing(0);
  layout->addWidget(slider_);
  layout->addSpacing(8);
  layout->addWidget(editor_);
  layout->addWidget(size_label);
  this->setLayout(layout);
  this->setContentsMargins(10, 0, 10, 0);
  // Same as TableComboBox
  this->setFixedSize(280, 36);
}

void PartitionSizeSlider::onEditorTextChanged(const QString& text) {
  bool ok;
  int value = text.toInt(&ok);
  if (ok) {
    const int tmp = static_cast<int>(maximum_size_ / kMebiByte);
    if(value > tmp){
      value = tmp;
      editor_->setText(QString::number(value));
    }

    // Block value-changed signal of slider to hold cursor position in editor.
    slider_->blockSignals(true);
    slider_->setValue(value);
    slider_->blockSignals(false);

    // Also emit valueChanged() signal.
    emit this->valueChanged(value * kMebiByte);
  }
}

void PartitionSizeSlider::onSliderValueChanged(int value) {
  editor_->setText(QString::number(value));

  // Emit valueChanged() signal.
  emit this->valueChanged(value * kMebiByte);
}

void PartitionSizeSlider::onEditorFinished()
{
    int value = slider_->value();
    editor_->setText(QString::number(value));
}

}  // namespace installer
