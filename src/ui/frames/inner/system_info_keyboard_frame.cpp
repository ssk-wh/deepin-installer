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

#include "ui/frames/inner/system_info_keyboard_frame.h"

#include <QDebug>
#include <QEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QScrollBar>

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/consts.h"
#include "ui/models/keyboard_layout_model.h"
#include "ui/models/keyboard_layout_variant_model.h"
#include "ui/utils/widget_util.h"
#include "ui/views/frameless_list_view.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/comment_label.h"

namespace installer {

namespace {

const int kLayoutWidth = 860;

}  // namespace

class SystemInfoKeyboardFramePrivate : public QObject
{
    Q_OBJECT

public:
    SystemInfoKeyboardFramePrivate(SystemInfoKeyboardFrame* parent);
    ~SystemInfoKeyboardFramePrivate();

private:
    void initUI();
    void initConnections();

    // Update variant list when new keyboard layout is selected.
    // Update system keyboard layout.
    // Emit layoutUpdate() signal.
    // Clear content of test_edit_.
    void onLayoutViewSelectionChanged(const QModelIndex& current,
                                      const QModelIndex& previous);

    // Update system keyboard layout when new layout variant is selected.
    // Clear content of test_edit_.
    void onVariantViewSelected(const QModelIndex& current,
                               const QModelIndex& previous);

    Q_DECLARE_PUBLIC(SystemInfoKeyboardFrame)
    SystemInfoKeyboardFrame* q_ptr;

    TitleLabel* title_label_ = nullptr;
    QLabel* guide_label_ = nullptr;
    FramelessListView* layout_view_ = nullptr;
    KeyboardLayoutModel* layout_model_ = nullptr;
    FramelessListView* variant_view_ = nullptr;
    KeyboardLayoutVariantModel* variant_model_ = nullptr;
    QLineEdit* test_edit_ = nullptr;
    NavButton* back_button_ = nullptr;
    QString current_locale_;
};

SystemInfoKeyboardFrame::SystemInfoKeyboardFrame(QWidget* parent)
    : QFrame(parent)
      , d_ptr(new SystemInfoKeyboardFramePrivate(this))
{
    setObjectName("system_info_keyboard_frame");
}

SystemInfoKeyboardFrame::~SystemInfoKeyboardFrame()
{
}

void SystemInfoKeyboardFrame::readConf() {
  // Load xkb config first.
  Q_D(SystemInfoKeyboardFrame);

  d->current_locale_ = ReadLocale();
  d->layout_model_->initLayout(d->current_locale_);

  const QString kb_layout = GetSettingsString("DI_LAYOUT");
  const QString kb_variant = GetSettingsString("DI_LAYOUT_VARIANT");
  const QString kb_default_layout = GetSettingsString(kSystemInfoDefaultKeyboardLayout);
  const QString kb_default_variant = GetSettingsString(kSystemInfoDefaultKeyboardLayoutVariant);
  const QString layout = kb_layout.isEmpty() ? kb_default_layout : kb_layout;
  const QString variant = kb_layout.isEmpty() ? kb_default_variant : kb_variant;

  if (layout.isEmpty()) {
    qWarning() << "Default keyboard layout is empty!";
    return;
  }

  const QModelIndex index = d->layout_model_->getLayoutByName(layout);
  if (index.isValid()) {
    // Select default layout.
    d->layout_view_->setCurrentIndex(index);
    const QModelIndex variant_index = d->variant_model_->getVariantIndexByName(variant);
    if (variant_index.isValid()) {
        d->variant_view_->setCurrentIndex(variant_index);
    }
    else {
        qWarning() << "Invalid default keyboard variant:" << variant;
    }
  } else {
    qWarning() << "Invalid default keyboard layout:" << layout;
  }
}

void SystemInfoKeyboardFrame::writeConf() {
  Q_D(SystemInfoKeyboardFrame);

  const QModelIndex layout_index = d->layout_view_->currentIndex();
  const QString layout = d->layout_model_->getLayoutName(layout_index);
  const QModelIndex variant_index = d->variant_view_->currentIndex();
  const QString variant = d->variant_model_->getVariantName(variant_index);

  // Model name of keyboard is empty. Variant name might be empty.
  // The first row in variant list is the default layout.
  if (variant_index.row() == 0) {
    WriteKeyboard("", layout, "");
  } else {
    WriteKeyboard("", layout, variant);
  }
}

void SystemInfoKeyboardFrame::changeEvent(QEvent* event) {
  Q_D(SystemInfoKeyboardFrame);

  if (event->type() == QEvent::LanguageChange) {
    d->title_label_->setText(tr("Select keyboard layout"));
    d->guide_label_->setText(tr("Select a proper keyboard layout"));
    d->test_edit_->setPlaceholderText(tr("Test here"));
    d->back_button_->setText(tr("Back"));

    const QString& locale = ReadLocale();
    d->layout_model_->initLayout(locale);

    int index = locale.indexOf('_');
    if (index >= 0){
        const QModelIndex modelIndex = d->layout_model_->getLayoutByName(locale.mid(index + 1).toLower());
        if (modelIndex.isValid()) {
            d->layout_view_->setCurrentIndex(modelIndex);
        }
    }
    else{
        qWarning() << "invalid locale:" << locale;
    }
  } else {
    QFrame::changeEvent(event);
  }
}

void SystemInfoKeyboardFramePrivate::initConnections() {
  connect(layout_view_->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &SystemInfoKeyboardFramePrivate::onLayoutViewSelectionChanged);
  connect(variant_view_->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &SystemInfoKeyboardFramePrivate::onVariantViewSelected);

  Q_Q(SystemInfoKeyboardFrame);

  connect(back_button_, &QPushButton::clicked,
          q, &SystemInfoKeyboardFrame::finished);
}

void SystemInfoKeyboardFramePrivate::initUI() {
  title_label_ = new TitleLabel(tr("Select keyboard layout"));
  guide_label_ = new CommentLabel(tr("Select a proper keyboard layout"));

  layout_view_ = new FramelessListView();
  layout_view_->setObjectName("layout_view");
  layout_model_ = new KeyboardLayoutModel(this);
  layout_view_->setModel(layout_model_);
  layout_view_->setFixedWidth(340);
  layout_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  layout_view_->setContextMenuPolicy(Qt::NoContextMenu);
  layout_view_->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  layout_view_->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

  variant_view_ = new FramelessListView();
  variant_view_->setObjectName("variant_view");
  variant_view_->setFixedWidth(519);
  variant_model_ = new KeyboardLayoutVariantModel(this);
  variant_view_->setModel(variant_model_);
  variant_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  variant_view_->setContextMenuPolicy(Qt::NoContextMenu);
  variant_view_->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  variant_view_->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

  QHBoxLayout* keyboard_layout = new QHBoxLayout();
  keyboard_layout->setContentsMargins(0, 0, 0, 0);
  keyboard_layout->setSpacing(0);
  keyboard_layout->addStretch();
  keyboard_layout->addWidget(layout_view_);
  // Add 1px margin between these two list views.
  keyboard_layout->addSpacing(1);
  keyboard_layout->addWidget(variant_view_);
  keyboard_layout->addStretch();

  QFrame* keyboard_wrapper = new QFrame();
  keyboard_wrapper->setObjectName("keyboard_wrapper");
  keyboard_wrapper->setFixedWidth(kLayoutWidth);
  keyboard_wrapper->setContentsMargins(0, 0, 0, 0);
  keyboard_wrapper->setLayout(keyboard_layout);
  QSizePolicy keyboard_size_policy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  keyboard_size_policy.setVerticalStretch(1);
  keyboard_wrapper->setSizePolicy(keyboard_size_policy);

  test_edit_ = new QLineEdit();
  test_edit_->setObjectName("test_edit");
  test_edit_->setPlaceholderText(tr("Test here"));
  test_edit_->setFixedSize(kLayoutWidth, 36);
  // Disable context menu.
  test_edit_->setContextMenuPolicy(Qt::NoContextMenu);

  back_button_ = new NavButton(tr("Next"));

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addSpacing(kMainLayoutSpacing + 40);
  layout->addWidget(title_label_, 0, Qt::AlignCenter);
  layout->addSpacing(kMainLayoutSpacing + 30);
  layout->addWidget(guide_label_, 0, Qt::AlignCenter);
  layout->addSpacing(kMainLayoutSpacing + 30);
  layout->addWidget(keyboard_wrapper, 0, Qt::AlignHCenter);
  layout->addSpacing(10);
  layout->addWidget(test_edit_, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addSpacing(kMainLayoutSpacing + 30);
  layout->addWidget(back_button_, 0, Qt::AlignHCenter);

  Q_Q(SystemInfoKeyboardFrame);

  q->setLayout(layout);
  q->setContentsMargins(0, 0, 0, 0);
  const QString style = ReadFile(":/styles/system_info_keyboard_frame.css");
  q->setStyleSheet(style);

  // Update style of list views, adding border radius.
  AppendStyleSheet(layout_view_, style);
  AppendStyleSheet(variant_view_, style);
}

void SystemInfoKeyboardFramePrivate::onLayoutViewSelectionChanged(
    const QModelIndex& current, const QModelIndex& previous) {
  Q_UNUSED(previous);
  test_edit_->clear();

  // Update variant list.
  variant_model_->setVariantList(layout_model_->getVariantList(current),
                                 current_locale_);

  // Scroll to top of variant view.
  variant_view_->scrollToTop();

  if (layout_view_->selectionModel()->selectedIndexes().size() > 0)  {
    // Select the default layout variant.
    variant_view_->setCurrentIndex(variant_model_->index(0));
  }
}

void SystemInfoKeyboardFramePrivate::onVariantViewSelected(
    const QModelIndex& current, const QModelIndex& previous) {
  Q_UNUSED(previous);

  // Clear content of test_edit_ when new layout variant is selected.
  test_edit_->clear();

  const QModelIndex layout_index = layout_view_->currentIndex();
  const QString layout = layout_model_->getLayoutName(layout_index);
  const QString variant = variant_model_->getVariantName(current);
  QString description;

  // The first row in variant list is the default layout.
  if (current.row() == 0) {
    description = layout_model_->getLayoutDescription(layout_index);
    if (!SetXkbLayout(layout)) {
      qWarning() << "SetXkbLayout() failed!" << layout;
    }
  } else {
    description = variant_model_->getVariantDescription(current);
    if (!SetXkbLayout(layout, variant)) {
      qWarning() << "SetXkbLayout() failed!" << layout << variant;
    }
  }

  Q_Q(SystemInfoKeyboardFrame);

  emit q->layoutUpdated(description);

  q->writeConf();
}

SystemInfoKeyboardFramePrivate::SystemInfoKeyboardFramePrivate(SystemInfoKeyboardFrame *parent)
    : q_ptr(parent)
{
    initUI();
    initConnections();
}

SystemInfoKeyboardFramePrivate::~SystemInfoKeyboardFramePrivate()
{
}

}  // namespace installer

#include "system_info_keyboard_frame.moc"

