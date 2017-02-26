// Copyright (c) 2016 Deepin Ltd. All rights reserved.
// Use of this source is governed by General Public License that can be found
// in the LICENSE file.

#include "ui/utils/widget_util.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDesktopWidget>
#include <QLayout>
#include <QLayoutItem>
#include <QMenu>

namespace installer {

namespace {

void WidgetTreeWalk(QWidget* root, int indent) {
  char prefix[indent+1];
  for (int i = 0; i < indent; i++) {
    prefix[i] = '*';
  }
  prefix[indent] = '\0';
  qDebug() << prefix << root;
  for (QObject* child : root->children()) {
    QWidget* item = dynamic_cast<QWidget*>(child);
    if (item) {
      WidgetTreeWalk(item, indent + 2);
    }
  }
}

}  // namespace

void AppendStyleSheet(QWidget* widget, const QString& style) {
  widget->setStyleSheet(widget->styleSheet() + style);
  widget->ensurePolished();
}

void ClearLayout(QLayout* layout) {
  for (QLayoutItem* item = layout->takeAt(0);
       item != nullptr;
       item = layout->takeAt(0)) {
    delete item->widget();
    delete item;
    item = nullptr;
  }
}

bool SetChildTransparent(QWidget* root, const QString& child_name) {
  for (QObject* child : root->children()) {
    if (child_name == child->metaObject()->className()) {
      QWidget* container = dynamic_cast<QWidget*>(child);
      if (container) {
        container->setAttribute(Qt::WA_TranslucentBackground, true);
        return true;
      }
    }
  }
  return false;
}

void ShowFullscreen(QWidget* widget) {
  widget->showFullScreen();
  // NOTE(xushaohua): If geometry of primary screen changes too fast, this
  // function may return false screen geometry.
  const QRect rect = qApp->desktop()->screenGeometry();
  widget->move(rect.topLeft());
  widget->setFixedSize(rect.size());
}

void SetQComboBoxTransparent(QComboBox* box) {
  if (!SetChildTransparent(box, "QComboBoxPrivateContainer")) {
    qWarning() << "SetQComboBoxTransparent() failed to find private container!";
  }
}

void SetQMenuTransparent(QMenu* menu) {
  if (!SetChildTransparent(menu, "QMenuPrivate")) {
    qWarning() << "SetQMenuTransparent() failed to find private container!";
  }
}

void WidgetTreeWalk(QWidget* root) {
  WidgetTreeWalk(root, 0);
}

}  // namespace installer