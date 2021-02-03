#ifndef CLI_INSTALL_DISK_INSUFFICIENT_FRAME_H
#define CLI_INSTALL_DISK_INSUFFICIENT_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

namespace installer {


class DiskSpaceInsufficientPrivate : public FrameInterfacePrivate
{

public:
    DiskSpaceInsufficientPrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void updateTs() override;
    bool validate() override;
    void show() override;
    void keyHandle() override;
    void setIsShow(bool isshow) override { m_isshow = isshow; }

private:
    NcursesButton* m_quitBtn;
    bool m_isshow;
};


class DiskSpaceInsufficient : public FrameInterface
{
    Q_OBJECT
public:
    DiskSpaceInsufficient(FrameInterface* parent);
    ~DiskSpaceInsufficient() override;

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

signals:
    void close();

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, DiskSpaceInsufficient)
};


}


#endif
