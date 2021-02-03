#ifndef INSTALL_NCURSES_CHECKBOX_H
#define INSTALL_NCURSES_CHECKBOX_H

#include <QObject>
#include "ui/ncurses_widgets/ncurses_windows_base.h"

namespace installer {

class NcursesTextBrower;

class NcursesCheckBox : public NCursesWindowBase
{
    Q_OBJECT

public:
    NcursesCheckBox(NCursesWindowBase* parent, const QString& text, int lines, int cols, int beginY, int beginX);
    NcursesCheckBox(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    virtual ~NcursesCheckBox() override;
    virtual void show() override;
    virtual void hide() override;
    virtual void setFocus(bool foucs) override;
    virtual void resizew(int newLines, int newColumns) override;
    void setText(const QString &title = "", const QString& text = "");
    void setSelect(bool select);
    bool isSelect(){ return m_select; }
    void setEnable(bool enable);
    void onKeyPress(int keyCode) override;
    QString title() const { return m_title; }
    QString text() const { return m_text; }
    bool isEnable(){ return m_enalble;}
    void setIsUseTitle(bool isuse){ m_isusetitle = isuse; }
    void clearText();
    void moveWindowTo(int y, int x);
    int getStrHeight(){ return m_strheight; }
    void setbkgd(chtype type);
    void setFocusStyle(chtype type);
    chtype getFocusStyle(){ return m_chtype_focus; }
    void setIsTitleUseCheckbox(bool type){ m_isTitleUseCheckbox = type; }

signals:
    void signal_SelectChange(bool select);

private:
    NcursesTextBrower* m_titlebrower;
    NcursesTextBrower* m_contentbrower;
    QString m_title;
    QString m_text;
    bool m_select;
    bool m_enalble;
    bool m_isusetitle;
    int m_selectshowpos;
    int m_strheight;
    bool m_isshow;
    bool m_isTitleUseCheckbox;
};

}

#endif // INSTALL_NCURSES_CHECKBOX_H
