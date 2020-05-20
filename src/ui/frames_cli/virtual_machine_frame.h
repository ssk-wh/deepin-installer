#ifndef CLI_INSTALL_VIRTUAL_MACHINE_FRAME_H
#define CLI_INSTALL_VIRTUAL_MACHINE_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

namespace installer {

class VirtualMachineFramePrivate : public FrameInterfacePrivate
{
public:
    VirtualMachineFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void updateTs() override;
};

class VirtualMachineFrame : public FrameInterface
{
    Q_OBJECT
public:
    VirtualMachineFrame(FrameInterface* parent);
    virtual ~VirtualMachineFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, VirtualMachineFrame)
};


}


#endif
