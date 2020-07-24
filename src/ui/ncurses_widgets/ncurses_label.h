#ifndef INSTALL_NCURSES_LABEL_H
#define INSTALL_NCURSES_LABEL_H

#include "ui/ncurses_widgets/ncurses_windows_base.h"
#include <QString>
namespace installer {

class NcursesLabel : public NCursesWindowBase
{
    Q_OBJECT
public:
    NcursesLabel(NCursesWindowBase* parent, const QString& text, int lines, int cols, int beginY, int beginX);
    NcursesLabel(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    virtual ~NcursesLabel(){}
    void setFocus(bool foucs) override;
    void setText(const QString& text);
    virtual void show() override;
    virtual void hide() override;
    QString text() const;
    void setAlignment(Qt::Alignment alignment);
    void adjustSizeByContext() override;
    void drawFoucs() override;

signals:
    void signal_OnFocus(bool focus);

private:
    NcursesLabel() : NCursesWindowBase(nullptr, 0, 0, 0, 0){}
private:
    QString m_text;
    Qt::Alignment m_alignment;
    bool m_isShow = false;
};

}

#endif // NCURSES_LABEL_H

