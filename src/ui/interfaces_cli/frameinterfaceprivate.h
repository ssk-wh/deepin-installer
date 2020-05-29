#ifndef CLI_INSTALL_FRAME_INTERFACE_PRIVATE_H
#define CLI_INSTALL_FRAME_INTERFACE_PRIVATE_H

#include <QObject>
#include "ui/ncurses_widgets/ncurses_windows_base.h"
#include "ui/ncurses_widgets/ncurses_button.h"
#include "service/settings_manager.h"
#include <QSharedPointer>
#include <QTimer>

namespace installer {

class FrameInterfacePrivate : public NCursesWindowBase
{
    Q_OBJECT
public:
    explicit FrameInterfacePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : NCursesWindowBase (parent, lines, cols, beginY, beginX)
    {
    }

    virtual ~FrameInterfacePrivate(){}
    virtual bool validate()
    {
        return true;
    }
    virtual void initUI() {

        setBackground(NcursesUtil::getInstance()->dialog_attr());

        this->drawShadow(true);
        this->box();

        QString strNext = QObject::tr("next");
        QString strBack = QObject::tr("back");

        /*m_pNextButton = new NcursesButton(this, strNext, buttonHeight,
                                        buttonWidth, begy() + height() - buttonHeight - 2, begx() + width() / 2 + buttonDistanceDelta);*/
        m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);

        m_pNextButton->drawShadow(true);
        m_pNextButton->box();
        m_pNextButton->setObjectName(strNext);
        m_pNextButton->setFocus(true);
        addChildWindows(m_pNextButton);
        connect(m_pNextButton, &NcursesButton::clicked, this, [=] {
            if (validate()) {
                emit next();
            }
        });

        /*m_pBackButton = new NcursesButton(this, strBack, buttonHeight,
                                          buttonWidth, begy() + height() - buttonHeight - 2, begx() + width() / 2 - buttonDistanceDelta - buttonWidth);*/

        if (canBack()) {
            m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
            m_pBackButton->drawShadow(true);
            m_pBackButton->box();
            m_pBackButton->setObjectName(strBack);

            addChildWindows(m_pBackButton);


            connect(m_pBackButton, &NcursesButton::clicked, this, &FrameInterfacePrivate::back);
        }
    }

    virtual void keyHandle() {
        while (!this->hidden()) {
                int key = getKey();
            switch (key) {
                case KEY_TAB:
                    switchChildWindowsFoucs();
                    show();
                break;
                default:
                    foreach (NCursesWindowBase* childWindow, m_childWindows) {
                        if (childWindow->isOnFoucs()) {
                             childWindow->onKeyPress(key);
                        }

                    }
                    onKeyPress(key);
            }
        }
    }

    virtual void keyEventTriger(int key) {
        switch (key) {
            case KEY_TAB:
                switchChildWindowsFoucs();
                show();
            break;
            case KEY_ESC:
               m_escCnt++;
               QTimer::singleShot(100, this, [=]{
                   if (m_escCnt == 1) {
                       Q_EMIT back();
                   } /*else if (m_escCnt == 2){
                       exit(0);
                   }*/

                   m_escCnt = 0;
               });
            break;
            default:
                foreach (NCursesWindowBase* childWindow, m_childWindows) {
                    if (childWindow->isOnFoucs()) {
                         childWindow->onKeyPress(key);
                    }

                }
                onKeyPress(key);
        }
    }

    virtual void updateTs()
    {
        QString strBack  = QObject::tr("back");
        QString strNext  = QObject::tr("next");
        int buttonHeight = 3;
        int buttonWidth  = 2;
        if (installer::ReadLocale() == "zh_CN") {
            buttonWidth = (std::max(strNext.length(), strBack.length()) * 2 + 4 * 2);
        } else {
            buttonWidth = (std::max(strNext.length(), strBack.length()) + 4);
        }

        if (m_pBackButton != nullptr) {
            m_pBackButton->erase();
            //m_pBackButton->resizew(buttonHeight, buttonWidth);
            m_pBackButton->resetBackground();
            m_pBackButton->box(ACS_VLINE,ACS_HLINE);
            m_pBackButton->setText(strBack);
            m_pBackButton->show();
        }

        if (m_pNextButton != nullptr) {
            m_pNextButton->erase();
            //m_pNextButton->resizew(buttonHeight, buttonWidth);
            m_pNextButton->resetBackground();
            m_pNextButton->box(ACS_VLINE,ACS_HLINE);
            m_pNextButton->setText(strNext);
            m_pNextButton->show();
        }
    }

    virtual void layout()
    {

    }

    virtual bool canBack() {
        return true;
    }

    virtual bool shouldDisplay() {
        return true;
    }

signals:
    void next();
    void back();
    void close();

protected:
    NcursesButton* m_pNextButton = nullptr;
    NcursesButton* m_pBackButton = nullptr;
    int m_escCnt = 0;
};

}

#endif // CLI_INSTALL_FRAME_INTERFACE_PRIVATE_H

