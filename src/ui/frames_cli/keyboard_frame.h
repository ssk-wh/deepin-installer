#ifndef INSTALL_KEYBOARD_FRAME_H
#define INSTALL_KEYBOARD_FRAME_H

#include "ui/interfaces_cli/frameinterfaceprivate.h"
#include "ui/interfaces_cli/frameinterface.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "sysinfo/keyboard.h"
namespace installer {

class KeyboardFrame;

class KeyboardFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
    friend KeyboardFrame;
public:
    KeyboardFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);

    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;

protected:
    void leftHandle() override;
    void rightHandle() override;

private:
    void readConf();
    void writeConf();
    void initLayout(const QString& locale);
    void onLayoutViewChanged(int currIndex);
    // Get index of layout with |name|.
     // Result might be invalid.
    int getLayoutByName(const QString& name);
    int getVariantIndexByName(const QString& name);
    XKbLayoutVariantList getVariantList(const int index);
    void setVariantList(const XKbLayoutVariantList& variant_list,
                        const QString& locale);
    // Get layout name at |index|.
    QString getLayoutName(const int index);
    // Get layout variant name at |index|.
    QString getVariantName(const int index);

    QString m_currentLocale;
    XkbConfig m_xkbCconfig;

    // Keyboard layout list sorted by description.
    XkbLayoutList m_layoutList;
    XKbLayoutVariantList m_variantList;
    int m_layoutIndex = -1;
    int m_variantIndex = -1;

    //NcursesLabel* m_titleLabel = nullptr;
    NcursesLabel* m_instructionsLabel = nullptr;
    NcursesListView* m_layoutView = nullptr;
    NcursesListView* m_variantView = nullptr;

    QString m_localeString = "";
};


class KeyboardFrame : public FrameInterface
{
    Q_OBJECT
public:
    KeyboardFrame(FrameInterface* parent);
    virtual ~KeyboardFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

protected:
    bool handle() override;
private:
    Q_DECLARE_PRIVATE_D(m_private, KeyboardFrame)
};

}
#endif
