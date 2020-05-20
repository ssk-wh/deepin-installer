#ifndef CLI_INSTALL_INSTALL_SCCUESS_FRAME_H
#define CLI_INSTALL_INSTALL_SCCUESS_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

namespace installer {

class NcursesTextBrower;

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

signals:
    void failFinished();
    void successFinished();

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

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, InstallSuccessFrame)
};


}


#endif
