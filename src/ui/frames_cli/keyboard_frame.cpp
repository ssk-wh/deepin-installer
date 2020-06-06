#include "keyboard_frame.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include <QCollator>

namespace installer {

void KeyboardFramePrivate::updateTs()
{
    if (!m_localeString.compare(installer::ReadLocale())) {
        return;
    }
    m_localeString = installer::ReadLocale();
    box(ACS_VLINE,ACS_HLINE);
    //m_titleLabel->setText(tr("Select keyboard layout"));
    printTitle(QObject::tr("Select keyboard layout"), width());
    m_instructionsLabel->setText(tr("Select a proper keyboard layout:"));
    readConf();
    FrameInterfacePrivate::updateTs();
    layout();
}

void KeyboardFramePrivate::layout()
{
    try {
        //m_titleLabel->adjustSizeByContext();
        //m_titleLabel->mvwin(begy(), begx() + (width() - m_titleLabel->width()) / 2);
        m_instructionsLabel->adjustSizeByContext();
        m_instructionsLabel->mvwin(begy() + 1, begx() + 10);
        m_layoutView->adjustSizeByContext();
        m_variantView->adjustSizeByContext();
        m_layoutView->mvwin(begy() + 3, begx() + (width() - (m_layoutView->width() + m_variantView->width() + 2)) / 2);
        m_variantView->mvwin(begy() + 3, begx() + (width() - (m_layoutView->width() + m_variantView->width() + 2)) / 2 + m_layoutView->width() + 2);
    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

KeyboardFramePrivate::KeyboardFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
{
    initUI();
    initConnection();
    //updateTs();
}

void KeyboardFramePrivate::initUI()
{
    try {
        FrameInterfacePrivate::initUI();
        //m_titleLabel = new NcursesLabel(this, 1, 25, begy(), begx());
        //m_titleLabel->setFocusEnabled(false);
        m_instructionsLabel = new NcursesLabel(this, 1, 35, begy(), begx());
        m_instructionsLabel->setFocusEnabled(false);
        m_layoutView = new NcursesListView(this, height() - 10, 20, begy(), begx());
        m_layoutView->setFocus(true);
        m_variantView = new NcursesListView(this, height() - 10, 20, begy(), begx());
    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }

}

void KeyboardFramePrivate::initConnection()
{
    connect(m_layoutView, &NcursesListView::selectChanged, this, &KeyboardFramePrivate::onLayoutViewChanged);
    connect(m_variantView, &NcursesListView::selectChanged, this, [=](int index) {
        if (index < m_variantView->size()) {
            m_variantIndex = index;
        }
    });
    connect(m_variantView, &NcursesListView::selectd, this, &KeyboardFramePrivate::next);

}

bool KeyboardFramePrivate::validate()
{
    writeConf();
    return true;
}

void KeyboardFramePrivate::readConf()
{
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

    const int index = getLayoutByName(layout);
    if (index >= 0) {
        // Select default layout.l
        m_layoutView->setCurrentIndex(index);
        const int variant_index = getVariantIndexByName(variant);
        if (variant_index >= 0) {
            m_variantView->setCurrentIndex(variant_index);
        }
        else {
            qWarning() << "Invalid default keyboard variant:" << variant;
        }
    } else {
        qWarning() << "Invalid default keyboard layout:" << layout;
    }
}

void KeyboardFramePrivate::writeConf()
{
    const QString layout = getLayoutName(m_layoutIndex);
    const QString variant = getVariantName(m_variantIndex);

    WriteKeyboard("", layout, variant);
}

void KeyboardFramePrivate::initLayout(const QString &locale)
{
    // Load xkb layout based on current locale.
     // Locale environment is setup in SelectLanguageFrame.
     m_xkbCconfig = GetXkbConfig(locale);
     m_layoutList = m_xkbCconfig.layout_list;


     // Append layout to its variant list.
     for (XkbLayout& layout : m_layoutList) {
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
     std::sort(m_layoutList.begin(), m_layoutList.end(),
               [&](const XkbLayout& a, const XkbLayout& b) -> bool {
                 return collator.compare(a.description, b.description) < 0;
               });

     QStringList strList;
     for(auto it = m_layoutList.cbegin(); it != m_layoutList.cend(); ++it) {
         strList << (*it).description;
     }
     m_layoutView->setList(strList);
}

void KeyboardFramePrivate::onLayoutViewChanged(int currIndex)
{
    if (m_layoutView->size() > currIndex) {
        m_layoutIndex = currIndex;
       // Update variant list.
        setVariantList(getVariantList(currIndex),  m_currentLocale);
        m_variantView->setCurrentIndex(0);
        m_variantView->show();
    }
}

int KeyboardFramePrivate::getLayoutByName(const QString &name)
{
    for (int row = 0; row < m_layoutList.length(); ++row) {
        if (m_layoutList.at(row).name == name) {
          return row;
        }
      }
      return -1;
}

int KeyboardFramePrivate::getVariantIndexByName(const QString &name)
{
    for (int row = 0; row < m_variantList.length(); row++) {
            if (m_variantList.at(row).name == name) {
                return row;
            }
        }
        return -1;
}

XKbLayoutVariantList KeyboardFramePrivate::getVariantList(const int index)
{
    if (index >= 0) {
        return m_layoutList.at(index).variant_list;
      } else {
        return XKbLayoutVariantList();
      }
}

void KeyboardFramePrivate::setVariantList(const XKbLayoutVariantList &variant_list, const QString &locale)
{
    Q_UNUSED(locale);

    m_variantList = variant_list;

    if (m_variantList.isEmpty()) {
    qCritical() << "VariantList is empty! We shall never reach here!";
    return;
    }

    // Sort variant list by description.
    const QLocale curr_locale(locale);
    QCollator collator(curr_locale);
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    // Sorting variant list by description, skipping the first item.
    std::sort(m_variantList.begin() + 1, m_variantList.end(),
            [&](const XkbLayoutVariant& a, const XkbLayoutVariant& b) -> bool {
              return collator.compare(a.description, b.description) < 0;
            });

    // Call this method resulting in emitting dataChanged() signal.

    QStringList strList;
    for(auto it = m_variantList.cbegin(); it!=m_variantList.cend(); ++it) {
      strList << (*it).description;
    }
    m_variantView->setList(strList);
}

QString KeyboardFramePrivate::getLayoutName(const int index)
{
    if (index >= 0) {
        return m_layoutList.at(index).name;
      } else {
        return QString();
      }
}

QString KeyboardFramePrivate::getVariantName(const int index)
{
    if (index >= 0) {
        return m_variantList.at(index).name;
      } else {
        return QString();
      }
}

KeyboardFrame::KeyboardFrame(FrameInterface *parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new KeyboardFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
}

KeyboardFrame::~KeyboardFrame()
{

}

bool KeyboardFrame::init()
{
    Q_D(KeyboardFrame);
    if (m_currState == FRAME_STATE_NOT_START) {
        d->readConf();
        m_private->layout();
        m_currState = FRAME_STATE_RUNNING;
    }
    return true;
}

QString KeyboardFrame::getFrameName()
{
    return "KeyboardFrame";
}

bool KeyboardFrame::handle()
{    
    return true;
}




}
