#ifndef CLI_INSTALL_LICENCE_FRAME_H
#define CLI_INSTALL_LICENCE_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"

namespace installer {

class NcursesTextBrower;
class NcursesCheckBox;
class LicenceFrame;
class NcursesLabel;

class LicenceFramePrivate : public FrameInterfacePrivate
{
    friend LicenceFrame;
    Q_OBJECT
public:
    LicenceFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    ~LicenceFramePrivate() override;

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    virtual void onKeyPress(int keyCode) override;

private:
    void setLicense(const QString text);
    void switchLicense();
    void browseLicense(int key);

private slots:
    void checkboxSelectChange(bool select);

private:
    NcursesLabel*      m_user_license_lab;
    NcursesLabel*      m_privacy_license;
    NcursesTextBrower* m_ncursesTextBrower;
    NcursesCheckBox*   m_NcursesCheckBox;
    NcursesLabel*      m_errorInfoLabel;
    QString            m_localeString;
    QString            m_show_license;
    bool               m_isshow;
};

class LicenceFrame : public FrameInterface
{
    Q_OBJECT

public:
    LicenceFrame(FrameInterface* parent);
    virtual ~LicenceFrame() override;

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

    QString getAbout() override{
        return   QString("<↑ ↓ ← →>%1  | <Tab>%2 | <Enter>%3 | <Space>%4")
                .arg(::QObject::tr("Select Item"))
                .arg(::QObject::tr("Change Field"))
                .arg(::QObject::tr("Confirm"))
                .arg(::QObject::tr("Select"));
    }

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, LicenceFrame)
};

}
#endif // CLI_INSTALL_LICENCE_FRAME_H
