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
    SystemInfoKeyboardFramePrivate(SystemInfoKeyboardFrame *ptr)
        : q_ptr(ptr)
    {}

    SystemInfoKeyboardFrame *q_ptr;
    Q_DECLARE_PUBLIC(SystemInfoKeyboardFrame)

private:
    void initConnections();
    void initUI();
    void updateTs();

    // Update variant list when new keyboard layout is selected.
    // Update system keyboard layout.
    // Emit layoutUpdate() signal.
    // Clear content of m_testEdit.
    void onLayoutViewSelectionChanged(const QModelIndex& current,
                                      const QModelIndex& previous);

    // Update system keyboard layout when new layout variant is selected.
    // Clear content of m_testEdit.
    void onVariantViewSelected(const QModelIndex& current,
                               const QModelIndex& previous);

    TitleLabel* m_titleLabel = new TitleLabel("");
    QLabel* m_guideLabel = new QLabel;
    FramelessListView* m_layoutView = new FramelessListView;
    KeyboardLayoutModel* m_layoutModel = new KeyboardLayoutModel;
    FramelessListView* m_variantView = new FramelessListView;
    KeyboardLayoutVariantModel* m_variantModel = new KeyboardLayoutVariantModel;
    QLineEdit* m_testEdit = new QLineEdit;
    NavButton* m_backButton = new NavButton;
    QString m_currentLocale;
};

SystemInfoKeyboardFrame::SystemInfoKeyboardFrame(QWidget* parent)
    : QFrame(parent)
    ,d_private(new SystemInfoKeyboardFramePrivate(this))
{
    this->setObjectName("system_info_keyboard_frame");

    d_private->m_currentLocale = "";
    d_private->initUI();
    d_private->initConnections();
    d_private->updateTs();
}

SystemInfoKeyboardFrame::~SystemInfoKeyboardFrame()
{

}

void SystemInfoKeyboardFramePrivate::updateTs() {
    m_titleLabel->setText(tr("Select keyboard layout"));
    m_guideLabel->setText(tr("Select a proper keyboard layout"));
    m_testEdit->setPlaceholderText(tr("Test here"));
    m_backButton = new NavButton(tr("Next"));
}

void SystemInfoKeyboardFrame::readConf() {
    Q_D(SystemInfoKeyboardFrame);

    // Load xkb config first.
    d->m_currentLocale = ReadLocale();
    d->m_layoutModel->initLayout(d->m_currentLocale);

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

    const QModelIndex index = d->m_layoutModel->getLayoutByName(layout);
    if (index.isValid()) {
        // Select default layout.l
        d->m_layoutView->setCurrentIndex(index);
        const QModelIndex variant_index = d->m_variantModel->getVariantIndexByName(variant);
        if (variant_index.isValid()) {
            d->m_variantView->setCurrentIndex(variant_index);
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

    const QModelIndex layout_index = d->m_layoutView->currentIndex();
    const QString layout = d->m_layoutModel->getLayoutName(layout_index);
    const QModelIndex variant_index = d->m_variantView->currentIndex();
    const QString variant = d->m_variantModel->getVariantName(variant_index);

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
        d->updateTs();
        d->m_backButton->setText(tr("Back"));

        const QString& locale = ReadLocale();
        d->m_layoutModel->initLayout(locale);

        int index = locale.indexOf('_');
        if (index >= 0){
            const QModelIndex modelIndex = d->m_layoutModel->getLayoutByName(locale.mid(index + 1).toLower());
            if (modelIndex.isValid()) {
                d->m_layoutView->setCurrentIndex(modelIndex);
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
    Q_Q(SystemInfoKeyboardFrame);

    connect(m_layoutView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SystemInfoKeyboardFramePrivate::onLayoutViewSelectionChanged);
    connect(m_variantView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SystemInfoKeyboardFramePrivate::onVariantViewSelected);

    connect(m_backButton, &QPushButton::clicked,
            q, &SystemInfoKeyboardFrame::finished);
}

void SystemInfoKeyboardFramePrivate::initUI() {
    Q_Q(SystemInfoKeyboardFrame);

    m_layoutView->setObjectName("layout_view");
    m_layoutView->setModel(m_layoutModel);
    m_layoutView->setFixedWidth(340);
    m_layoutView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_layoutView->setContextMenuPolicy(Qt::NoContextMenu);
    m_layoutView->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_layoutView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_variantView->setObjectName("variant_view");
    m_variantView->setFixedWidth(519);
    m_variantView->setModel(m_variantModel);
    m_variantView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_variantView->setContextMenuPolicy(Qt::NoContextMenu);
    m_variantView->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_variantView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    QHBoxLayout* keyboard_layout = new QHBoxLayout();
    keyboard_layout->setContentsMargins(0, 0, 0, 0);
    keyboard_layout->setSpacing(0);
    keyboard_layout->addStretch();
    keyboard_layout->addWidget(m_layoutView);
    // Add 1px margin between these two list views.
    keyboard_layout->addSpacing(1);
    keyboard_layout->addWidget(m_variantView);
    keyboard_layout->addStretch();

    QFrame* keyboard_wrapper = new QFrame();
    keyboard_wrapper->setObjectName("keyboard_wrapper");
    keyboard_wrapper->setFixedWidth(kLayoutWidth);
    keyboard_wrapper->setContentsMargins(0, 0, 0, 0);
    keyboard_wrapper->setLayout(keyboard_layout);
    QSizePolicy keyboard_size_policy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    keyboard_size_policy.setVerticalStretch(1);
    keyboard_wrapper->setSizePolicy(keyboard_size_policy);

    m_testEdit->setObjectName("test_edit");
    m_testEdit->setFixedSize(kLayoutWidth, 36);
    // Disable context menu.
    m_testEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addSpacing(kMainLayoutSpacing + 40);
    layout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    layout->addSpacing(kMainLayoutSpacing + 30);
    layout->addWidget(m_guideLabel, 0, Qt::AlignCenter);
    layout->addSpacing(kMainLayoutSpacing + 30);
    layout->addWidget(keyboard_wrapper, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(m_testEdit, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addSpacing(kMainLayoutSpacing + 30);
    layout->addWidget(m_backButton, 0, Qt::AlignHCenter);

    q->setLayout(layout);
    q->setContentsMargins(0, 0, 0, 0);
    const QString style = ReadFile(":/styles/system_info_keyboard_frame.css");
    q->setStyleSheet(style);

    // Update style of list views, adding border radius.
    AppendStyleSheet(m_layoutView, style);
    AppendStyleSheet(m_variantView, style);
}

void SystemInfoKeyboardFramePrivate::onLayoutViewSelectionChanged(
        const QModelIndex& current, const QModelIndex& previous) {
    Q_UNUSED(previous);

    m_testEdit->clear();

    // Update variant list.
    m_variantModel->setVariantList(m_layoutModel->getVariantList(current),
                                   m_currentLocale);

    // Scroll to top of variant view.
    m_variantView->scrollToTop();

    if (m_layoutView->selectionModel()->selectedIndexes().size() > 0)  {
        // Select the default layout variant.
        m_variantView->setCurrentIndex(m_variantModel->index(0));
    }
}

void SystemInfoKeyboardFramePrivate::onVariantViewSelected(
        const QModelIndex& current, const QModelIndex& previous) {
    Q_Q(SystemInfoKeyboardFrame);

    Q_UNUSED(previous);

    // Clear content of m_testEdit when new layout variant is selected.
    m_testEdit->clear();

    const QModelIndex layout_index = m_layoutView->currentIndex();
    const QString layout = m_layoutModel->getLayoutName(layout_index);
    const QString variant = m_variantModel->getVariantName(current);
    QString description;

    // The first row in variant list is the default layout.
    if (current.row() == 0) {
        description = m_layoutModel->getLayoutDescription(layout_index);
        if (!SetXkbLayout(layout)) {
            qWarning() << "SetXkbLayout() failed!" << layout;
        }
    } else {
        description = m_variantModel->getVariantDescription(current);
        if (!SetXkbLayout(layout, variant)) {
            qWarning() << "SetXkbLayout() failed!" << layout << variant;
        }
    }
    emit q->layoutUpdated(description);

    q->writeConf();
}

}  // namespace installer

#include "system_info_keyboard_frame.moc"
