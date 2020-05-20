#ifndef CLI_INSTALL_PRIVILETE_ERROR_FRAME_H
#define CLI_INSTALL_PRIVILETE_ERROR_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

namespace installer {


class PrivilegeErrorFramePrivate : public FrameInterfacePrivate
{
public:
    PrivilegeErrorFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void updateTs() override;
    bool validate() override;
};

class PrivilegeErrorFrame : public FrameInterface
{
public:
    explicit PrivilegeErrorFrame(FrameInterface* parent);

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;
protected:
    bool handle() override;
private:
    Q_DECLARE_PRIVATE_D(m_private, PrivilegeErrorFrame)
};


}

#endif
