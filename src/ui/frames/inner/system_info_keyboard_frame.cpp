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
#include <QScrollArea>
#include <QStandardItemModel>
#include <QCollator>
#include <algorithm>
#include <QPushButton>

#include <DListView>
#include <DStandardItem>
#include <DFrame>
#include <DVerticalLine>

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/consts.h"
#include "ui/models/keyboard_layout_model.h"
#include "ui/models/keyboard_layout_variant_model.h"
#include "ui/utils/widget_util.h"
#include "ui/views/frameless_list_view.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/tip_list_view.h"
#include "ui/interfaces/frameinterfaceprivate.h"


DWIDGET_USE_NAMESPACE

namespace installer {

namespace {

const int kLayoutWidth = 556;
const int kLeftViewWidth = 190 - 20 - 1;
const int kRightViewWidth = kLayoutWidth - 190 - 1 - 20 - 1;

}  // namespace

class SystemInfoKeyboardFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    SystemInfoKeyboardFramePrivate(SystemInfoKeyboardFrame *pattern):
        FrameInterfacePrivate(pattern),
        q_ptr(qobject_cast<SystemInfoKeyboardFrame*>(pattern))
    {}

    SystemInfoKeyboardFrame *q_ptr;
    Q_DECLARE_PUBLIC(SystemInfoKeyboardFrame)

private:
    void initConnections();
    void initUI();
    void updateTs();

    void readConf();
    void writeConf();

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

    // Get index of layout with |name|.
    // Result might be invalid.
    QModelIndex getLayoutByName(const QString& name);

    // Get layout name at |index|.
    QString getLayoutName(const QModelIndex& index);

    // Read xkb layout list with |locale|.
    void initLayout(const QString& locale);

    // Get variant list at |index|.
    XKbLayoutVariantList getVariantList(const QModelIndex& index);

    // Get layout description at |index|.
    QString getLayoutDescription(const QModelIndex& index);

    QModelIndex getVariantIndexByName(const QString& name);

    // Get layout variant name at |index|.
    QString getVariantName(const QModelIndex& index);

    // Set variant list and emit dataChanged() signal.
    void setVariantList(const XKbLayoutVariantList& variant_list,
                        const QString& locale);

    // Get description at |index|.
    QString getVariantDescription(const QModelIndex& index);

    TitleLabel* m_titleLabel = new TitleLabel("");
    QLabel* m_guideLabel = new CommentLabel;
    QStandardItemModel* m_layoutModel = nullptr;
    QStandardItemModel* m_variantModel = nullptr;
    QString m_currentLocale;
    XkbConfig xkb_config_;

    DListView* m_layoutView = new TipListView;
    DListView* m_variantView = new TipListView;

    // The user last checked item.
    DStandardItem* m_lastItem = nullptr;
    DStandardItem* m_lastItemVar = nullptr;

    QModelIndex m_lastMode;
    QModelIndex m_lastModeVar;

    // Keyboard layout list sorted by description.
    XkbLayoutList layout_list_;
    XKbLayoutVariantList variant_list_;
    XkbModelList model_list;

    // The user last checked both left and right view item.
    DStandardItem* m_lastChangedItem = nullptr;
    int m_lastItemRow = -1;
};

SystemInfoKeyboardFrame::SystemInfoKeyboardFrame(FrameProxyInterface *parent)
    : FrameInterface(parent)
    , d_private(new SystemInfoKeyboardFramePrivate(this))
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

void SystemInfoKeyboardFrame::init()
{
    d_private->readConf();
}

void SystemInfoKeyboardFrame::finished()
{
    d_private->writeConf();
}

bool SystemInfoKeyboardFrame::shouldDisplay() const {
    return !GetSettingsBool(kSkipSystemKeyboardPage);
}

void SystemInfoKeyboardFrame::layoutViewScrolle(int testindex)
{
    QModelIndex current = d_private->m_layoutView->currentIndex();//d->localeIndex(d->lang_.locale);
    if (current.isValid()) {
        if(((current.row() + testindex) < 0) || ((current.row() + testindex) >= d_private->m_layoutView->count())) {
            return;
        }

        QScrollArea *testarea = this->findChild<QScrollArea *>("LeftSourceScrollArea");
        double testitemheight = (d_private->m_layoutView->height() * 1.0) / d_private->m_layoutView->count();
        int testlistviewheight = d_private->m_layoutView->height();
        int testareaheight = testarea->height();
        double testareaheightneed = testareaheight - (testlistviewheight % testareaheight);
        double testcurpage = (current.row() * testitemheight) / testareaheight;
        double testafterpage = ((current.row() + testindex) * testitemheight) / testareaheight;

        if (testafterpage < testcurpage) {
            int testvaluetouse = testlistviewheight - (d_private->m_layoutView->count() * testitemheight - testcurpage * testareaheight);
            if (testvaluetouse > testareaheightneed) {
                double testisdoscroll = d_private->m_layoutView->count() * testitemheight - testafterpage * testareaheight;
                double testlimit = (testareaheight * (int)(((d_private->m_layoutView->count() * testitemheight) / testareaheight - testcurpage)) + testareaheight);
                if (testisdoscroll > testlimit) {
                    int testvalue = testarea->verticalScrollBar()->value() + testareaheight * -1;
                    testarea->verticalScrollBar()->setValue(testvalue);
                }
            } else {
                int testvalue = testarea->verticalScrollBar()->value() + (testvaluetouse * -1);
                testarea->verticalScrollBar()->setValue(testvalue);
            }
        } else if (testafterpage > testcurpage) {
            int testvaluetouse = testlistviewheight - (testcurpage * testareaheight);
            if (testvaluetouse > testareaheightneed) {
                double testisdoscroll = testafterpage * testareaheight;
                double testlimit = (testareaheight * (int)(testcurpage) + testareaheight);
                if (testisdoscroll > testlimit) {
                    int testvalue = testarea->verticalScrollBar()->value() + testareaheight;
                    testarea->verticalScrollBar()->setValue(testvalue);
                }
            } else {
                int testvalue = testarea->verticalScrollBar()->value() + testvaluetouse;
                testarea->verticalScrollBar()->setValue(testvalue);
            }
        }

        d_private->m_layoutView->setCurrentIndex(current.siblingAtRow(current.row() + testindex));
    }
}

void SystemInfoKeyboardFrame::variantViewScrolle(int testindex)
{
    QModelIndex current = d_private->m_variantView->currentIndex();//d->localeIndex(d->lang_.locale);
    if (current.isValid()) {
        if(((current.row() + testindex) < 0) || ((current.row() + testindex) >= d_private->m_variantView->count())) {
            return;
        }

        QScrollArea *testarea = this->findChild<QScrollArea *>("RightSourceScrollArea");
        double testitemheight = (d_private->m_variantView->height() * 1.0) / d_private->m_variantView->count();
        int testlistviewheight = d_private->m_variantView->height();
        int testareaheight = testarea->height();
        double testareaheightneed = testareaheight - (testlistviewheight % testareaheight);
        double testcurpage = (current.row() * testitemheight) / testareaheight;
        double testafterpage = ((current.row() + testindex) * testitemheight) / testareaheight;

        if (testafterpage < testcurpage) {
            int testvaluetouse = testlistviewheight - (d_private->m_variantView->count() * testitemheight - testcurpage * testareaheight);
            if (testvaluetouse > testareaheightneed) {
                double testisdoscroll = d_private->m_variantView->count() * testitemheight - testafterpage * testareaheight;
                double testlimit = (testareaheight * (int)(((d_private->m_variantView->count() * testitemheight) / testareaheight - testcurpage)) + testareaheight);
                if (testisdoscroll > testlimit) {
                    int testvalue = testarea->verticalScrollBar()->value() + testareaheight * -1;
                    testarea->verticalScrollBar()->setValue(testvalue);
                }
            } else {
                int testvalue = testarea->verticalScrollBar()->value() + (testvaluetouse * -1);
                testarea->verticalScrollBar()->setValue(testvalue);
            }
        } else if (testafterpage > testcurpage) {
            int testvaluetouse = testlistviewheight - (testcurpage * testareaheight);
            if (testvaluetouse > testareaheightneed) {
                double testisdoscroll = testafterpage * testareaheight;
                double testlimit = (testareaheight * (int)(testcurpage) + testareaheight);
                if (testisdoscroll > testlimit) {
                    int testvalue = testarea->verticalScrollBar()->value() + testareaheight;
                    testarea->verticalScrollBar()->setValue(testvalue);
                }
            } else {
                int testvalue = testarea->verticalScrollBar()->value() + testvaluetouse;
                testarea->verticalScrollBar()->setValue(testvalue);
            }
        }

        d_private->m_variantView->setCurrentIndex(current.siblingAtRow(current.row() + testindex));
    }
}

QModelIndex SystemInfoKeyboardFramePrivate::getLayoutByName(const QString& name) {
  for (int row = 0; row < layout_list_.length(); ++row) {
    if (layout_list_.at(row).name == name) {
      return m_layoutModel->index(row,0);
    }
  }
  return QModelIndex();
}

QString SystemInfoKeyboardFramePrivate::getLayoutName(const QModelIndex& index) {
  if (index.isValid()) {
    return layout_list_.at(index.row()).name;
  } else {
    return QString();
  }
}

void SystemInfoKeyboardFramePrivate::initLayout(const QString& locale) {
  // Load xkb layout based on current locale.
  // Locale environment is setup in SelectLanguageFrame.
  xkb_config_ = GetXkbConfig(locale);
  layout_list_ = xkb_config_.layout_list;

  m_layoutModel->clear();
  m_lastItem = nullptr;
  m_lastChangedItem = nullptr;
  m_lastItemVar = nullptr;
  m_lastItemRow = -1;

  // Append layout to its variant list.
  for (XkbLayout& layout : layout_list_) {
    XkbLayoutVariant variant;
    variant.name = layout.name;
    variant.description = layout.description;
    variant.short_description = layout.short_description;
    variant.language_list = layout.language_list;
    layout.variant_list.prepend(variant);
  }

  // Sort layout list by description.
  // Perform localized comparison.
  const QLocale curr_locale(locale);
  QCollator collator(curr_locale);
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  std::sort(layout_list_.begin(), layout_list_.end(),
            [&](const XkbLayout& a, const XkbLayout& b) -> bool {
              return collator.compare(a.description, b.description) < 0;
            });

  for(auto it = layout_list_.cbegin(); it != layout_list_.cend(); ++it) {
      DStandardItem *item = new DStandardItem((*it).description);
      m_layoutModel->appendRow(item);
  }

  m_layoutView->update();
}

XKbLayoutVariantList SystemInfoKeyboardFramePrivate::getVariantList(
    const QModelIndex& index) {
  if (index.isValid()) {
    return layout_list_.at(index.row()).variant_list;
  } else {
    return XKbLayoutVariantList();
  }
}

QString SystemInfoKeyboardFramePrivate::getLayoutDescription(
    const QModelIndex& index) {
  if (index.isValid()) {
    return layout_list_.at(index.row()).description;
  } else {
    return QString();
  }
}

QModelIndex SystemInfoKeyboardFramePrivate::getVariantIndexByName(const QString& name)
{
    for (int row = 0; row < variant_list_.length(); row++) {
        if (variant_list_.at(row).name == name) {
            return m_variantModel->index(row,0);
        }
    }
    return QModelIndex();
}

void SystemInfoKeyboardFramePrivate::setVariantList(
    const XKbLayoutVariantList& variant_list, const QString& locale) {
  Q_UNUSED(locale);

  variant_list_ = variant_list;

  if (variant_list_.isEmpty()) {
    qCritical() << "VariantList is empty! We shall never reach here!";
    return;
  }

  // Sort variant list by description.
  const QLocale curr_locale(locale);
  QCollator collator(curr_locale);
  collator.setCaseSensitivity(Qt::CaseInsensitive);

  // Sorting variant list by description, skipping the first item.
  std::sort(variant_list_.begin() + 1, variant_list_.end(),
            [&](const XkbLayoutVariant& a, const XkbLayoutVariant& b) -> bool {
              return collator.compare(a.description, b.description) < 0;
            });

  // Call this method resulting in emitting dataChanged() signal.

  for(auto it = variant_list_.cbegin(); it!=variant_list_.cend(); ++it) {
      DStandardItem* item = new DStandardItem((*it).description);
      m_variantModel->appendRow(item);
  }

  m_variantView->update();

  onVariantViewSelected(m_variantModel->index(0, 0), m_variantModel->index(0, 0));
}

QString SystemInfoKeyboardFramePrivate::getVariantName(
    const QModelIndex& index) {
  if (index.isValid()) {
    return variant_list_.at(index.row()).name;
  } else {
    return QString();
  }
}

QString SystemInfoKeyboardFramePrivate::getVariantDescription(
    const QModelIndex& index) {
  if (index.isValid()) {
    return variant_list_.at(index.row()).description;
  } else {
    return QString();
  }
}

void SystemInfoKeyboardFramePrivate::updateTs() {
    m_titleLabel->setText(::QObject::tr("Set Keyboard Layout"));
    m_guideLabel->setText(::QObject::tr("Select a proper keyboard layout"));
    nextButton->setText(::QObject::tr("Next"));
}

void SystemInfoKeyboardFramePrivate::readConf() {
    // Load xkb config first.
    m_currentLocale = ReadLocale();
    initLayout(m_currentLocale);

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

    const QModelIndex index = getLayoutByName(layout);
    if (index.isValid()) {
        // Select default layout.l
        m_layoutView->setCurrentIndex(index);
        const QModelIndex variant_index = getVariantIndexByName(variant);
        if (variant_index.isValid()) {
            m_variantView->setCurrentIndex(variant_index);
        }
        else {
            qWarning() << "Invalid default keyboard variant:" << variant;
            m_variantView->setCurrentIndex(m_variantModel->index(0, 0));
        }
    } else {
        qWarning() << "Invalid default keyboard layout:" << layout;
    }

}

void SystemInfoKeyboardFramePrivate::writeConf() {

    const QModelIndex layout_index = m_layoutView->currentIndex();
    const QString layout = getLayoutName(layout_index);
    const QModelIndex variant_index = m_variantView->currentIndex();
    const QString variant = getVariantName(variant_index);

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
        d->initLayout(ReadLocale());
    } else {
        FrameInterface::changeEvent(event);
    }
}

void SystemInfoKeyboardFrame::showEvent(QShowEvent *event)
{
    Q_D(SystemInfoKeyboardFrame);
    QStringList localeList;
    localeList << ReadLocale() << GetSettingsString(kSelectLanguageDefaultLocale);

    for (const QString& locale : localeList) {
        int index = locale.indexOf('_');
        if (index >= 0){
            const QModelIndex modelIndex = d->getLayoutByName(locale.mid(index + 1).toLower());
            if (modelIndex.isValid()) {
                d->m_layoutView->setCurrentIndex(modelIndex);
                break;
            }
        }
        else{
            qWarning() << "invalid locale:" << locale;
        }
    }

    this->setCurentFocus(d_private->nextButton);

    return FrameInterface::showEvent(event);
}

bool SystemInfoKeyboardFrame::focusSwitch()
{
    if (m_current_focus_widget == nullptr) {
        this->setCurentFocus(d_private->nextButton);
    } else if (d_private->nextButton == m_current_focus_widget) {
        this->setCurentFocus(d_private->m_layoutView);
    } else if (d_private->m_layoutView == m_current_focus_widget) {
        this->setCurentFocus(d_private->nextButton);
    } else if (d_private->m_variantView == m_current_focus_widget) {
        this->setCurentFocus(d_private->nextButton);
    }

    return true;
}

bool SystemInfoKeyboardFrame::doSpace()
{
    return true;
}

bool SystemInfoKeyboardFrame::doSelect()
{
    if (d_private->nextButton == m_current_focus_widget) {
        emit d_private->nextButton->clicked();
    }
    return true;
}

bool SystemInfoKeyboardFrame::directionKey(int keyvalue)
{
    int testindexstep = 0;
    switch (keyvalue) {
    case Qt::Key_Up: {
            testindexstep--;
            if (d_private->m_layoutView == m_current_focus_widget) {
                layoutViewScrolle(testindexstep);
            } else if (d_private->m_variantView == m_current_focus_widget) {
                variantViewScrolle(testindexstep);
            }
        }
        break;
    case Qt::Key_Down: {
            testindexstep++;
            if (d_private->m_layoutView == m_current_focus_widget) {
                layoutViewScrolle(testindexstep);
            } else if (d_private->m_variantView == m_current_focus_widget) {
                variantViewScrolle(testindexstep);
            }
        }
        break;
    case Qt::Key_Left: {
            if ((d_private->m_layoutView == m_current_focus_widget) || (d_private->m_variantView == m_current_focus_widget)) {
                this->setCurentFocus(d_private->m_layoutView);
            }
        }
        break;
    case Qt::Key_Right: {
            if ((d_private->m_layoutView == m_current_focus_widget) || (d_private->m_variantView == m_current_focus_widget)) {
                this->setCurentFocus(d_private->m_variantView);
            }
        }
        break;
    }

    return true;
}

void SystemInfoKeyboardFramePrivate::initConnections() {
//    Q_Q(SystemInfoKeyboardFrame);

    connect(m_layoutView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SystemInfoKeyboardFramePrivate::onLayoutViewSelectionChanged);
    connect(m_variantView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SystemInfoKeyboardFramePrivate::onVariantViewSelected);
}

void SystemInfoKeyboardFramePrivate::initUI() {
    m_layoutView->setObjectName("layout_view");
    m_layoutModel = new QStandardItemModel(m_layoutView);
    m_layoutView->setModel(m_layoutModel);
    m_layoutView->setFixedWidth(kLeftViewWidth);
    m_layoutView->setItemSize(QSize(kLeftViewWidth, 40));
    m_layoutView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_layoutView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_layoutView->setContextMenuPolicy(Qt::NoContextMenu);
    m_layoutView->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_layoutView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_layoutView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_layoutView->setEditTriggers(QListView::NoEditTriggers);
    m_layoutView->setIconSize(QSize(32, 32));
    m_layoutView->setResizeMode(QListView::Adjust);
    m_layoutView->setMovement(QListView::Static);
    m_layoutView->setSelectionMode(QListView::NoSelection);
    m_layoutView->setFrameShape(QFrame::NoFrame);
    //m_layoutView->setFocusPolicy(Qt::TabFocus);
    m_layoutView->setStyleSheet("QWidget#layout_view::item::focus{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");

    m_variantView->setObjectName("variant_view");
    m_variantView->setFixedWidth(kRightViewWidth);
    m_variantModel = new QStandardItemModel(m_variantView);
    m_variantView->setModel(m_variantModel);
    m_variantView->setItemSize(QSize(kRightViewWidth, 40));
    m_variantView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_variantView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_variantView->setContextMenuPolicy(Qt::NoContextMenu);
    m_variantView->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_variantView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_variantView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_variantView->setEditTriggers(QListView::NoEditTriggers);
    m_variantView->setIconSize(QSize(32, 32));
    m_variantView->setResizeMode(QListView::Adjust);
    m_variantView->setMovement(QListView::Static);
    m_variantView->setSelectionMode(QListView::NoSelection);
    m_variantView->setFrameShape(QFrame::NoFrame);
    //m_variantView->setFocusPolicy(Qt::TabFocus);
    m_variantView->setStyleSheet("QWidget#variant_view::item::focus{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(5, 5, 15, 0);
    leftLayout->setSpacing(0);
    leftLayout->addWidget(m_layoutView);
    QFrame *leftListViewWrap = new QFrame;
    leftListViewWrap->setLayout(leftLayout);

    QScrollArea *leftSourceScrollArea = new QScrollArea;
    leftSourceScrollArea->setObjectName("LeftSourceScrollArea");
    leftSourceScrollArea->setWidgetResizable(true);
    //leftSourceScrollArea->setFocusPolicy(Qt::TabFocus);
    leftSourceScrollArea->setFrameStyle(QFrame::NoFrame);
    leftSourceScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    leftSourceScrollArea->setContentsMargins(0, 0, 0, 0);
    leftSourceScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    leftSourceScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftSourceScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    leftSourceScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    leftSourceScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    leftSourceScrollArea->setWidget(leftListViewWrap);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(5, 5, 15, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_variantView);
    QFrame *rightListViewWrap =  new QFrame;
    rightListViewWrap->setLayout(rightLayout);

    QScrollArea *rightSourceScrollArea = new QScrollArea;
    rightSourceScrollArea->setObjectName("RightSourceScrollArea");
    rightSourceScrollArea->setWidgetResizable(true);
    //rightSourceScrollArea->setFocusPolicy(Qt::TabFocus);
    rightSourceScrollArea->setFrameStyle(QFrame::NoFrame);
    rightSourceScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    rightSourceScrollArea->setContentsMargins(0, 0, 0, 0);
    rightSourceScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    rightSourceScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightSourceScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    rightSourceScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    rightSourceScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    rightSourceScrollArea->setWidget(rightListViewWrap);

    DVerticalLine* dVerticalLine = new DVerticalLine;

    QHBoxLayout* keyboard_layout = new QHBoxLayout();
    keyboard_layout->setContentsMargins(0, 0, 0, 0);
    keyboard_layout->setSpacing(0);
    keyboard_layout->addStretch();
    keyboard_layout->addWidget(leftSourceScrollArea, 0, Qt::AlignRight);
    keyboard_layout->addWidget(dVerticalLine);
    keyboard_layout->addWidget(rightSourceScrollArea, 0, Qt::AlignLeft);
    keyboard_layout->addStretch();

    DFrame* keyboard_wrapper = new DFrame;
    keyboard_wrapper->setObjectName("keyboard_wrapper");
    keyboard_wrapper->setFixedWidth(kLayoutWidth);
    keyboard_wrapper->setFrameRounded(true);
    keyboard_wrapper->setContentsMargins(1, 1, 1, 1);
    keyboard_wrapper->setLayout(keyboard_layout);
    QSizePolicy keyboard_size_policy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    keyboard_size_policy.setVerticalStretch(1);
    keyboard_wrapper->setSizePolicy(keyboard_size_policy);

    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    centerLayout->addSpacing(30);
    centerLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    centerLayout->addSpacing(20);
    centerLayout->addWidget(m_guideLabel, 0, Qt::AlignCenter);
    centerLayout->addSpacing(15);
    centerLayout->addWidget(keyboard_wrapper, 0, Qt::AlignHCenter);
    centerLayout->addStretch();
    centerLayout->addSpacing(15);
}

void SystemInfoKeyboardFramePrivate::onLayoutViewSelectionChanged(
        const QModelIndex& current, const QModelIndex& previous) {
    Q_UNUSED(previous);

    m_variantModel->clear();
    // Update variant list.
    setVariantList(getVariantList(current),  m_currentLocale);

    if (m_lastMode.isValid()) {
        DStandardItem* tmpItem = dynamic_cast<DStandardItem* >(m_layoutModel->item(m_lastMode.row()));
        if (tmpItem != nullptr) {
            tmpItem->setCheckState(Qt::Unchecked);
        }
    }

    m_lastMode = current;

    DStandardItem* item = dynamic_cast<DStandardItem* >(m_layoutModel->item(current.row()));
    if (item != nullptr) {
        item->setCheckState(Qt::Checked);
    }

    // Scroll to top of variant view.
    m_variantView->scrollToTop();
}

void SystemInfoKeyboardFramePrivate::onVariantViewSelected(
        const QModelIndex& current, const QModelIndex& previous) {
    Q_UNUSED(previous);

    if (m_lastModeVar.isValid()) {
        DStandardItem* tmpItem = dynamic_cast<DStandardItem* >(m_variantModel->item(m_lastModeVar.row()));
        if (tmpItem != nullptr) {
            tmpItem->setCheckState(Qt::Unchecked);
        }
    }

    m_lastModeVar = current;

    DStandardItem* item = dynamic_cast<DStandardItem* >(m_variantModel->item(current.row()));
    if (item != nullptr) {
        item->setCheckState(Qt::Checked);
    }

    m_lastChangedItem = dynamic_cast<DStandardItem* >
            (m_layoutModel->item(m_layoutView->currentIndex().row()));

    const QModelIndex layout_index = m_layoutView->currentIndex();
    const QString layout = getLayoutName(layout_index);
    const QString variant = getVariantName(current);
    QString description;

    // The first row in variant list is the default layout.
    if (current.row() == 0) {
        description = getLayoutDescription(layout_index);
        if (!SetXkbLayout(layout)) {
            qWarning() << "SetXkbLayout() failed!" << layout;
        }
    } else {
        description = getVariantDescription(current);
        if (!SetXkbLayout(layout, variant)) {
            qWarning() << "SetXkbLayout() failed!" << layout << variant;
        }
    }
}

}  // namespace installer

#include "system_info_keyboard_frame.moc"
