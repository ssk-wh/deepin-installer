#ifndef CLI_INSTALL_FRAME_INTERFACE_PRIVATE_H
#define CLI_INSTALL_FRAME_INTERFACE_PRIVATE_H

#include <QObject>
#include "ui/ncurses_widgets/ncurses_windows_base.h"
#include "ui/ncurses_widgets/ncurses_button.h"
#include "service/settings_manager.h"
#include "ui/ncurses_widgets/ncurses_quit.h"
#include <QSharedPointer>
#include <QTimer>

namespace  {
const int kKeyLeft = 260;
const int kKeyRight = 261;
}

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

        QString strNext = ::QObject::tr("Next");
        QString strBack = ::QObject::tr("Back");

        m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);

        m_pNextButton->drawShadow(true);
        m_pNextButton->box();
        m_pNextButton->setObjectName(strNext);
        //m_pNextButton->setFocus(true);
        addChildWindows(m_pNextButton);
        connect(m_pNextButton, &NcursesButton::clicked, this, [=] {
            if (validate()) {
                emit next();
            }
        });

        if (canBack()) {
            m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
            m_pBackButton->drawShadow(true);
            m_pBackButton->box();
            m_pBackButton->setObjectName(strBack);
            addChildWindows(m_pBackButton);
            connect(m_pBackButton, &NcursesButton::clicked, this, &FrameInterfacePrivate::back);
        }
    }

    void show() override {
        NCursesWindowBase::show();
        if (m_quit) m_quit->hide();
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
        if (m_quit && m_quit->isOnFoucs()) {
            m_quit->keyHandle(key);
        } else {
            switch (key) {
                case KEY_TAB: tabKeyHandle(); break;
                case KEY_ESC: escKeyHandle(); break;
                case kKeyUp: upHandle(); break;
                case kKeyDown: downHandle(); break;
                case kKeyLeft: leftHandle(); break;
                case kKeyRight: rightHandle(); break;
                default: defaultHandle(key);
            }
        }

    }

    virtual void updateTs()
    {
        QString strBack  = ::QObject::tr("Back");
        QString strNext  = ::QObject::tr("Next");
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

protected:
    virtual void tabKeyHandle() {
        defaultHandle(KEY_TAB);
    }

    void escKeyHandle() {
//        m_escCnt++;
//        QTimer::singleShot(0, this, [=]{
//            if (m_escCnt == 1) {
////                backHandle();
////            } else if (m_escCnt == 2){
                quitHandle();
//            }

//            m_escCnt = 0;
//        });
    }

    virtual void quitHandle() {
        this->hide();
        if (!m_quit) {
            m_quit = new NcursesQuit(this, this->height(), this->width(), begy(), begx(), false, true);
            m_quit->setFocusEnabled(false);
            connect(m_quit, &NcursesQuit::cancel, this, [=]{
                this->show();
                m_quit->hide();
            });
        }
        m_quit->show();
        m_quit->setFocus(true);
    }

    virtual void backHandle() {
        Q_EMIT back();
    }

    virtual void rightHandle() {
        defaultHandle(kKeyRight);
    }

    virtual void leftHandle() {
        defaultHandle(kKeyLeft);
    }

    virtual void upHandle() {
        defaultHandle(kKeyUp);
    }

    virtual void downHandle() {
        defaultHandle(kKeyDown);
    }

private:
    void defaultHandle(int key) {
        foreach (NCursesWindowBase* childWindow, m_childWindows) {
            if (childWindow->isOnFoucs()) {
                 childWindow->onKeyPress(key);
            }
        }
        onKeyPress(key);
    }

signals:
    void next();
    void back();
    void close();


protected:
    NcursesQuit *  m_quit = nullptr;
    NcursesButton* m_pNextButton = nullptr;
    NcursesButton* m_pBackButton = nullptr;
    int m_escCnt = 0;
};

}

#endif // CLI_INSTALL_FRAME_INTERFACE_PRIVATE_H

