#ifndef CLI_INSTALL_INSTALL_PROCESS_FRAME_H
#define CLI_INSTALL_INSTALL_PROCESS_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

class QTimer;

namespace installer {

class HooksManager;
class NcursesProgressBar;
class NcursesTextBrower;

class InstallProcessFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    InstallProcessFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    ~InstallProcessFramePrivate();

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    virtual void onKeyPress(int keyCode);

    // Handles error state
    void onHooksErrorOccurred();
    // Handles successful installation.
    void onHooksFinished();
    void onProgressUpdate(int progress);
    void startInstall();
signals:
    void signal_timeout();

private slots:
    void slot_timeout();

protected:
    void backHandle() override {}
    void quitHandle() override {}

private:
    bool failed_;
    int m_progressvalue;
    NcursesProgressBar* m_NcursesProgressBar;
    NcursesTextBrower* m_NcursesTextBrower;
    HooksManager* hooks_manager_;
    QThread* hooks_manager_thread_;
    bool m_isshow;
    QTimer* m_processTimer;
};


class InstallProcessFrame : public FrameInterface
{
    Q_OBJECT
public:
    InstallProcessFrame(FrameInterface* parent);
    virtual ~InstallProcessFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;
    void startInstall();

    QString getAbout() override{
        return   QString("");
    }

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, InstallProcessFrame)
};


}


#endif
