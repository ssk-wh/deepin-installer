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
#include <QPushButton>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <DListView>
#include <DStandardItem>
#include <DSysInfo>
#include <QScrollArea>

#include "base/file_util.h"
#include "base/translator.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/language_delegate.h"
#include "ui/delegates/style_delegate.h"
#include "ui/frames/consts.h"
#include "ui/models/language_list_model.h"
#include "ui/views/frameless_list_view.h"
#include "ui/delegates/user_agreement_delegate.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/comment_label.h"
#include "service/system_language.h"
#include "service/language_manager.h"

DWIDGET_USE_NAMESPACE

DCORE_USE_NAMESPACE

namespace installer {

class SelectLanguageFramePrivate : public QObject {
    Q_OBJECT
public:
    explicit SelectLanguageFramePrivate(SelectLanguageFrame* frame) : q_ptr(frame) {}

    SelectLanguageFrame*   q_ptr               = nullptr;
    QTranslator*           current_translator_ = nullptr;
    QCheckBox*             accept_license_     = nullptr;
    QLabel*                license_label_      = nullptr;
    QCheckBox*             accept_experience_     = nullptr;
    QLabel*                experience_label_      = nullptr;
    QLabel*                oem_and_label_      = nullptr;
    QLabel*                oem_license_label_  = nullptr;
    QLabel*                sub_title_label_    = nullptr;
    QPushButton*           next_button_        = nullptr;
    QStandardItemModel*    m_languageModel     = nullptr;
    LanguageItem           lang_;  // Current selected language.
    UserAgreementDelegate* user_license_delegate_ = nullptr;
    LanguageList           lang_list_;
    DStandardItem*         m_lastItem          = nullptr;
    DListView*             m_languageView      = nullptr;
    QList<QString>         m_languageSortList;

    void initConnections();
    void initUI();
    void updateTranslator(const QString& locale);
    void updateTs();
    void onLanguageListSelected(const QModelIndex& current);
    void onAccpetLicenseChanged(bool enable);
    void appendLanguageitem();
    void readLanguageSortFile();

    // Get language item at |index|.
    LanguageItem languageItemAt(const QModelIndex& index);

    // Get index with |locale|.
    QModelIndex localeIndex(const QString& locale);

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
    d_private->readLanguageSortFile();
    d_private->appendLanguageitem();
    d_private->m_languageView->installEventFilter(this);
}

SelectLanguageFrame::~SelectLanguageFrame() {

}

LanguageItem SelectLanguageFramePrivate::languageItemAt(const QModelIndex& index) {
    return lang_list_.at(index.row());
}

QModelIndex SelectLanguageFramePrivate::localeIndex(const QString& locale)  {
    for (auto it = lang_list_.cbegin(); it != lang_list_.cend(); ++it) {
        if ((*it).locale == locale) {
            return m_languageModel->index((it - lang_list_.cbegin()), 0);
        }
    }
    return QModelIndex();
}

void SelectLanguageFrame::readConf() {
  Q_D(SelectLanguageFrame);
// Select default locale
  const QString di_locale = GetSettingsString("DI_LOCALE");
  const QString default_locale = GetSettingsString(kSelectLanguageDefaultLocale);
  const QString locale = di_locale.isEmpty() ? default_locale : di_locale;
  const QModelIndex index = d->localeIndex(locale);
  if (index.isValid()) {
    d->lang_ = d->languageItemAt(index);
    d->m_languageView->setCurrentIndex(index);
    emit d->m_languageView->clicked(index);
  }
}

void SelectLanguageFrame::writeConf() {
  Q_D(SelectLanguageFrame);
    WriteLocale(d->lang_.locale);

    if (d->accept_experience_->checkState() != Qt::Unchecked) {
        WriteUserExperience(true);
    } else {
        WriteUserExperience(false);
    }
}

bool SelectLanguageFrame::isChecked()
{
    bool enable = GetSettingsBool(kSystemInfoDisableLicense) || d_private->accept_license_->isChecked();
    return !d_private->lang_.name.isEmpty() && enable;
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

    if (event->type() == QEvent::KeyPress && d->accept_license_->isChecked() && !d->lang_.name.isEmpty()) {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() == Qt::Key_Return || key_event->key() == Qt::Key_Enter) {
            // Simulate button click event.
            emit requestApplyLanguage();
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

    if (obj == d->experience_label_) {
        switch (event->type()) {
            case QEvent::MouseButtonRelease: emit requestShowUserExperience(); break;
            case QEvent::Enter: setCursor(QCursor(Qt::PointingHandCursor)); break;
            case QEvent::Leave: setCursor(QCursor(Qt::ArrowCursor)); break;
            default: break;
        }
    }

    if (event->type() == QEvent::KeyPress && obj == d->m_languageView) {
        QKeyEvent* key = dynamic_cast<QKeyEvent*>(event);
        switch (key->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                QModelIndex index = d->m_languageView->currentIndex();

                Q_D(SelectLanguageFrame);
                d->onLanguageListSelected(index);

                break;
        }
    }

    return QObject::eventFilter(obj, event);
}

void SelectLanguageFrame::showEvent(QShowEvent *event)
{
    Q_D(SelectLanguageFrame);

    d->m_languageView->setFocus();

    QFrame::showEvent(event);
}

void SelectLanguageFramePrivate::initConnections()
{
    connect(m_languageView, &DListView::clicked, this,
            &SelectLanguageFramePrivate::onLanguageListSelected);
    connect(accept_license_, &QCheckBox::clicked, this, &SelectLanguageFramePrivate::onAccpetLicenseChanged);
}

void SelectLanguageFramePrivate::appendLanguageitem()
{
    lang_list_ = GetLanguageList();

    if (!m_languageSortList.isEmpty()) {
        qSort(lang_list_.begin(), lang_list_.end(), [&] (LanguageItem& left, LanguageItem& right) {
            return m_languageSortList.indexOf(left.name) < m_languageSortList.indexOf(right.name);
        });
    }
    else {
        qCritical() << "language sort list is empty";
    }

    m_languageModel->clear();

    for(auto it = lang_list_.cbegin(); it!=lang_list_.cend(); ++it) {
        if (m_languageSortList.indexOf((*it).name) < 0) {
            qInfo() << (*it).name << " is not exist in language sort file";
        }

        DStandardItem *item = new DStandardItem((*it).local_name);
        m_languageModel->appendRow(item);
    }
}

void SelectLanguageFramePrivate::readLanguageSortFile()
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(GetLanguageSort().toUtf8(), &error);

    if (error.error == QJsonParseError::NoError){
        if(doc.isArray()){
            QJsonArray array = doc.array();
            for (auto it = array.begin(); it != array.end(); ++it) {
                m_languageSortList << it->toString();
            }
        }
    }
}

void SelectLanguageFramePrivate::initUI() {
    Q_Q(SelectLanguageFrame);

    QLabel* logo_label = new QLabel();
    logo_label->setPixmap(QPixmap(installer::GetVendorLogo()));

    QLabel* title_label = new QLabel("Select Language");
    title_label->setObjectName("title_label");
    title_label->setWordWrap(false);
    title_label->setAlignment(Qt::AlignHCenter);

    sub_title_label_ = new CommentLabel();
    sub_title_label_->setWordWrap(false);
    sub_title_label_->setAlignment(Qt::AlignHCenter);

    m_languageView = new DListView();
    m_languageView->setFocusPolicy(Qt::TabFocus);
    m_languageView->setEditTriggers(QListView::NoEditTriggers);
    m_languageView->setIconSize(QSize(32, 32));
    m_languageView->setResizeMode(QListView::Adjust);
    m_languageView->setMovement(QListView::Static);
    m_languageView->setSelectionMode(QListView::NoSelection);
    m_languageView->setFrameShape(QFrame::NoFrame);
    m_languageView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_languageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_languageView->setContextMenuPolicy(Qt::NoContextMenu);
    m_languageView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_languageView->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_languageModel=new QStandardItemModel(m_languageView);
    m_languageView->setModel(m_languageModel);
    m_languageView->installEventFilter(q);

    QScrollArea* languageArea = StyleDelegate::area(m_languageView);

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

    QFrame* licenseWidget = new QFrame;
    licenseWidget->setLayout(license_layout);
    licenseWidget->setVisible(!GetSettingsBool(kSystemInfoDisableLicense));

    accept_experience_ = new QCheckBox;
    accept_experience_->setCheckable(true);
    accept_experience_->setChecked(false);
    accept_experience_->setFocusPolicy(Qt::NoFocus);

    experience_label_ = new QLabel;
    experience_label_->setObjectName("LicenseLabel");
    experience_label_->installEventFilter(q);

    QHBoxLayout* experience_layout = new QHBoxLayout;
    experience_layout->setMargin(0);
    experience_layout->setSpacing(5);
    experience_layout->addStretch();
    experience_layout->addWidget(accept_experience_);
    experience_layout->addWidget(experience_label_);

    experience_layout->addStretch();

    QFrame* experienceWidget = new QFrame;
    experienceWidget->setLayout(experience_layout);
    experienceWidget->setVisible(!GetSettingsBool(kSystemInfoDisableExperience));

    QVBoxLayout *user_layout = new QVBoxLayout;
    user_layout->addWidget(experienceWidget, 0, Qt::AlignLeft);
    user_layout->addSpacing(10);
    user_layout->addWidget(licenseWidget, 0, Qt::AlignLeft);
    QFrame* userFrame = new QFrame;
    userFrame->setLayout(user_layout);

    next_button_ = new QPushButton;
    next_button_->setEnabled(GetSettingsBool(kSystemInfoDisableLicense));

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kMainLayoutSpacing);
    layout->addSpacing(30);
    layout->addWidget(logo_label, 0, Qt::AlignCenter);
    layout->addWidget(title_label, 0, Qt::AlignCenter);
    layout->addWidget(sub_title_label_, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(languageArea, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(userFrame, 0, Qt::AlignHCenter);

    layout->addSpacing(20);

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
    QPalette palette;
    palette.setColor(QPalette::Text, QColor(66, 154, 216));

    accept_license_->setText(::QObject::tr("I have read and agree to the"));
    license_label_->setText(::QObject::tr("%1 Software End User License Agreement").arg(DSysInfo::productType() == DSysInfo::Deepin ? ::QObject::tr("Deepin") : ::QObject::tr("UOS")));
    license_label_->setPalette(palette);

    if (oem_license_label_ != nullptr) {
        oem_and_label_->setText(::QObject::tr("and"));
        QString     localeName{ installer::ReadLocale() };
        LicenseItem license_item =
            user_license_delegate_->getPrimaryAdaptiveLicense(localeName);
        if (license_item.isValid()) {
            oem_license_label_->setText(license_item.basicName());
        }
    }

    accept_experience_->setText(::QObject::tr("Agree to the"));
    experience_label_->setText(::QObject::tr("User Experience Program License Agreement"));
    experience_label_->setPalette(palette);
    sub_title_label_->setText(::QObject::tr("Select Language"));
}

void SelectLanguageFramePrivate::onLanguageListSelected(const QModelIndex& current)
{
    Q_Q(SelectLanguageFrame);

    if (current.isValid()) {
        // Update locale on-the-fly.
        const LanguageItem language_item = lang_list_.at(current.row());
        DStandardItem* item = dynamic_cast<DStandardItem* >(m_languageModel->item(current.row()));

        if (item == m_lastItem) {
            return;
        }

        item->setCheckState(Qt::Checked);

        if (m_lastItem) {
            m_lastItem->setCheckState(Qt::Unchecked);
        }

        m_lastItem = item;

        this->updateTranslator(language_item.locale);
        lang_ = language_item;
        q->writeConf();
        emit q->timezoneUpdated(language_item.timezone);
        onAccpetLicenseChanged(GetSettingsBool(kSystemInfoDisableLicense) || accept_license_->isChecked());
    }
}

void SelectLanguageFramePrivate::onAccpetLicenseChanged(bool enable) {
    if (enable) {
        m_languageView->setFocusPolicy(Qt::NoFocus);
        q_ptr->setFocus();
    } else {
        m_languageView->setFocusPolicy(Qt::TabFocus);
    }
    emit q_ptr->requestNextButtonEnable(enable && !lang_.name.isEmpty());
}

}  // namespace installer

#include "select_language_frame.moc"
