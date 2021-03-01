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

#include "ui/widgets/comment_label.h"
#include "base/file_util.h"

#include <QFont>

namespace installer {

CommentLabel::CommentLabel(QWidget *parent)
    : CommentLabel(QString(), parent)
{
}

CommentLabel::CommentLabel(const QString& text, QWidget* parent)
  : QLabel(text, parent) {
  setObjectName("comment_label");

  setFixedWidth(540);
  setAlignment(Qt::AlignCenter);
  setWordWrap(true);

  QPalette pe = palette();
  pe.setColor(QPalette::Text, QColor("#526a7f"));
  setPalette(pe);
}

}  // namespace installer
