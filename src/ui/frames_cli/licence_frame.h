#ifndef CLI_INSTALL_LICENCE_FRAME_H
#define CLI_INSTALL_LICENCE_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

namespace installer {

class NcursesTextBrower;
class NcursesCheckBox;
class LicenceFrame;
class LicenceFramePrivate : public FrameInterfacePrivate
{
    friend LicenceFrame;
    Q_OBJECT
public:
    LicenceFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    ~LicenceFramePrivate();

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    virtual void onKeyPress(int keyCode);

private:
    NcursesTextBrower* m_ncursesTextBrower;
    NcursesCheckBox*   m_NcursesCheckBox;
};

class LicenceFrame : public FrameInterface
{
    Q_OBJECT

public:
    LicenceFrame(FrameInterface* parent);
    virtual ~LicenceFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, LicenceFrame)
};

}
#endif // CLI_INSTALL_LICENCE_FRAME_H