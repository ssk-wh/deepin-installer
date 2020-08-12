#ifndef CLI_INSTALL_INSTALL_SCCUESS_FRAME_H
#define CLI_INSTALL_INSTALL_SCCUESS_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

namespace installer {

class NcursesTextBrower;
class SaveLogFrame;

class InstallSuccessFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    InstallSuccessFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    ~InstallSuccessFramePrivate();

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    void onKeyPress(int keyCode) override;
    virtual void keyEventTriger(int keycode) override;
    void setValue();

signals:
    void failFinished();
    void successFinished();
    void keyEventTrigerSignal(int keycode);
    void showChildSignal(int index);

private slots:
    void doBackBtnClicked();
    void doNextBtnClicked();

private:
    NcursesTextBrower* m_installresultTextBrower;
    QString m_installSuccessTitle;
    QString m_installSuccessInfoTitle;
    QString m_installSuccessInfoDes;
    QString m_installSuccessInfoTodo;
    QString m_installFailedTitle;
    QString m_installFailedInfoTitle;
    QString m_installFailedInfoDes;
    bool m_isshow;
    int m_currentchoicetype = -1;//-1-main page 0-Save Log page
};

class InstallSuccessFrame : public FrameInterface
{
    Q_OBJECT
public:
    InstallSuccessFrame(FrameInterface* parent);
    virtual ~InstallSuccessFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

private slots:
    void showChildSlot(int index);

protected:
    bool handle() override;

private:
    SaveLogFrame* m_savelogframe = nullptr;
    Q_DECLARE_PRIVATE_D(m_private, InstallSuccessFrame)
};


}


#endif
