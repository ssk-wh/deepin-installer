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

#include "ui/frames/inner/select_language_frame.h"

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QTranslator>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QScrollBar>

#include "base/file_util.h"
#include "base/translator.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/language_delegate.h"
#include "ui/frames/consts.h"
#include "ui/models/language_list_model.h"
#include "ui/views/frameless_list_view.h"
#include "ui/widgets/nav_button.h"
#include "ui/delegates/user_agreement_delegate.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/comment_label.h"

#include "service/language_manager.h"

#include "service/language_manager.h"

namespace installer {

class SelectLanguageFramePrivate : public QObject {
    Q_OBJECT
public:
    explicit SelectLanguageFramePrivate(SelectLanguageFrame* frame) : q_ptr(frame) {}

    SelectLanguageFrame*   q_ptr                 = nullptr;
    QTranslator*           current_translator_ = nullptr;
    FramelessListView*     language_view_      = nullptr;
    LanguageListModel*     language_model_     = nullptr;
    QCheckBox*             accept_license_     = nullptr;
    QLabel*                license_label_      = nullptr;
    QLabel*                oem_and_label_      = nullptr;
    QLabel*                oem_license_label_  = nullptr;
    QLabel*                sub_title_label_    = nullptr;
    NavButton*             next_button_        = nullptr;
    LanguageItem           lang_;  // Current selected language.
    UserAgreementDelegate* user_license_delegate_ = nullptr;

    void initConnections();
    void initUI();
    void updateTranslator(const QString& locale);
    void updateTs();
    void onLanguageListSelected(const QModelIndex& current);
    void onAccpetLicenseChanged(bool enable);

    Q_DECLARE_PUBLIC(SelectLanguageFrame);
};

SelectLanguageFrame::SelectLanguageFrame(UserAgreementDelegate* delegate, QWidget* parent)
    : QFrame(parent)
    , d_private(new SelectLanguageFramePrivate(this))
{
    d_private->current_translator_    = new QTranslator;
    d_private->user_license_delegate_ = delegate;
    this->setObjectName("select_language_frame");

    d_private->initUI();
    d_private->initConnections();

    d_private->language_view_->installEventFilter(this);
}

SelectLanguageFrame::~SelectLanguageFrame() {

}

void SelectLanguageFrame::readConf() {
  Q_D(SelectLanguageFrame);
// Select default locale
  const QString di_locale = GetSettingsString("DI_LOCALE");
  const QString default_locale = GetSettingsString(kSelectLanguageDefaultLocale);
  const QString locale = di_locale.isEmpty() ? default_locale : di_locale;
  const QModelIndex index = d->language_model_->localeIndex(locale);
  if (index.isValid()) {
    d->lang_ = d->language_model_->languageItemAt(index);
    d->language_view_->setCurrentIndex(index);
    emit d->language_view_->clicked(index);
  }
}

void SelectLanguageFrame::writeConf() {
  Q_D(SelectLanguageFrame);
  WriteLocale(d->lang_.locale);
}

void SelectLanguageFrame::changeEvent(QEvent* event) {
    Q_D(SelectLanguageFrame);

    if (event->type() == QEvent::LanguageChange) {
        return d->updateTs();
    }
    else {
        QFrame::changeEvent(event);
    }
}

bool SelectLanguageFrame::eventFilter(QObject* obj, QEvent* event) {
    Q_D(SelectLanguageFrame);

    if (event->type() == QEvent::KeyPress && d->next_button_->isEnabled()) {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() == Qt::Key_Return || key_event->key() == Qt::Key_Enter) {
            // Simulate button click event.
            d->next_button_->click();
            return true;
        }
    }

    if (obj == d->license_label_) {
        switch (event->type()) {
            case QEvent::MouseButtonRelease: emit requestShowUserLicense(); break;
            case QEvent::Enter: setCursor(QCursor(Qt::PointingHandCursor)); break;
            case QEvent::Leave: setCursor(QCursor(Qt::ArrowCursor)); break;
            default: break;
        }
    }

    if (d->oem_license_label_ != nullptr && obj == d->oem_license_label_) {
        switch (event->type()) {
            case QEvent::MouseButtonRelease: emit requestShowOemUserLicense(); break;
            case QEvent::Enter: setCursor(QCursor(Qt::PointingHandCursor)); break;
            case QEvent::Leave: setCursor(QCursor(Qt::ArrowCursor)); break;
            default: break;
        }
    }

    return QObject::eventFilter(obj, event);
}

void SelectLanguageFrame::showEvent(QShowEvent *event)
{
    Q_D(SelectLanguageFrame);

    d->language_view_->setFocus();

    QFrame::showEvent(event);
}

void SelectLanguageFramePrivate::initConnections()
{
    Q_Q(SelectLanguageFrame);

    connect(language_view_, &QListView::clicked, this,
            &SelectLanguageFramePrivate::onLanguageListSelected);

    connect(
        language_view_->selectionModel(), &QItemSelectionModel::currentChanged, q,
        [=](const QModelIndex& current, const QModelIndex& previous) {
            // Skip first signal
            if (current == language_view_->model()->index(0, 0) && !previous.isValid()) {
                return;
            }
            Q_UNUSED(previous);
            emit language_view_->clicked(current);
        });
    connect(next_button_, &QPushButton::clicked, q, &SelectLanguageFrame::finished);
    connect(accept_license_, &QCheckBox::clicked, this, &SelectLanguageFramePrivate::onAccpetLicenseChanged);
}

void SelectLanguageFramePrivate::initUI() {
    Q_Q(SelectLanguageFrame);

    QLabel* logo_label = new QLabel();
    logo_label->setPixmap(installer::renderPixmap(GetVendorLogo()));

    QLabel* title_label = new QLabel("Select system language");
    title_label->setObjectName("title_label");
    title_label->setWordWrap(false);
    title_label->setAlignment(Qt::AlignHCenter);

    sub_title_label_ = new CommentLabel();
    sub_title_label_->setWordWrap(false);
    sub_title_label_->setAlignment(Qt::AlignHCenter);
    LanguageManager::translator(sub_title_label_, &CommentLabel::setText, TranslatorType::SelectLanguageSubTitle);

    language_view_ = new FramelessListView();
    language_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    language_view_->setContextMenuPolicy(Qt::NoContextMenu);
    language_view_->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    language_view_->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    language_model_ = new LanguageListModel(language_view_);
    language_view_->setModel(language_model_);

    accept_license_ = new QCheckBox;
    accept_license_->setCheckable(true);
    accept_license_->setChecked(false);
    accept_license_->setFocusPolicy(Qt::NoFocus);

    license_label_ = new QLabel;
    license_label_->setObjectName("LicenseLabel");
    license_label_->installEventFilter(q);

    if (user_license_delegate_->isLicenseDirExists()) {
        oem_and_label_ = new QLabel;
        oem_and_label_->setObjectName("OemAndLabel");
        oem_license_label_ = new QLabel;
        oem_license_label_->setObjectName("LicenseLabel");
        oem_license_label_->installEventFilter(q);
    }

    QHBoxLayout* license_layout = new QHBoxLayout;
    license_layout->setMargin(0);
    license_layout->setSpacing(5);
    license_layout->addStretch();
    license_layout->addWidget(accept_license_);
    license_layout->addWidget(license_label_);

    if (nullptr != oem_license_label_) {
        license_layout->addWidget(oem_and_label_);
        license_layout->addWidget(oem_license_label_);
    }

    license_layout->addStretch();
    next_button_ = new NavButton(tr("Next"));
    next_button_->setEnabled(false);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kMainLayoutSpacing);
    layout->addSpacing(30);
    layout->addWidget(logo_label, 0, Qt::AlignCenter);
    layout->addWidget(title_label, 0, Qt::AlignCenter);
    layout->addWidget(sub_title_label_, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(language_view_, 0, Qt::AlignHCenter);
    layout->addSpacing(20);
    layout->addLayout(license_layout);

    layout->addSpacing(20);
    layout->addWidget(next_button_, 0, Qt::AlignCenter);

    q->setLayout(layout);
    q->setContentsMargins(0, 0, 0, 0);
    q->setStyleSheet(ReadFile(":/styles/select_language_frame.css"));

    updateTs();
}

void SelectLanguageFramePrivate::updateTranslator(const QString& locale) {
  if (!lang_.locale.isEmpty()) {
    // Remove the old translator if it is loaded.
    qApp->removeTranslator(current_translator_);
  }
  const QString locale_file(GetLocalePath(locale));
//  qDebug() << current_translator_->
  if (current_translator_->load(locale_file)) {
    if (!qApp->installTranslator(current_translator_)) {
      qWarning() << "Failed to update ui language at:" << locale_file;
    }
  } else {
    qWarning() << "Failed to load locale file:" << locale_file;
  }
}

void SelectLanguageFramePrivate::updateTs()
{
    next_button_->setText(tr("Next"));
    accept_license_->setText(tr("I have read and agree to"));
    license_label_->setText(tr("Deepin Software End User License Agreement"));

    if (oem_license_label_ != nullptr) {
        oem_and_label_->setText(tr("and"));
        QString     localeName{ installer::ReadLocale() };
        LicenseItem license_item =
            user_license_delegate_->getPrimaryAdaptiveLicense(localeName);
        if (license_item.isValid()) {
            oem_license_label_->setText(license_item.basicName());
        }
    }
}

void SelectLanguageFramePrivate::onLanguageListSelected(const QModelIndex& current)
{
    Q_Q(SelectLanguageFrame);

    if (current.isValid()) {
        // Update locale on-the-fly.
        const LanguageItem language_item = language_model_->languageItemAt(current);
        this->updateTranslator(language_item.locale);
        lang_ = language_item;
        q->writeConf();
        emit q->timezoneUpdated(language_item.timezone);
        onAccpetLicenseChanged(accept_license_->isChecked());
    }
}

void SelectLanguageFramePrivate::onAccpetLicenseChanged(bool enable) {
    next_button_->setEnabled(enable && !lang_.name.isEmpty());
}

}  // namespace installer

#include "select_language_frame.moc"
