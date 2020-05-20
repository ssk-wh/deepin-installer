#ifndef CLI_INSTALL_COMPONENT_FRAME_H
#define CLI_INSTALL_COMPONENT_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"
#include <QMap>

namespace installer {

class NcursesLabel;
class NcursesCheckBoxList;
struct ComponentInfo;
class ComponentStruct;

class InstallComponentFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    InstallComponentFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    ~InstallComponentFramePrivate();

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;

    void initInfoList();
    void writeInfoList();
    virtual void onKeyPress(int keyCode);

private slots:
    void slot_KeyTriger(int keycode, int listtype);

private:
    NcursesLabel* m_tiltleLabel;
    NcursesLabel* m_firstSubTiltleLabel;
    NcursesLabel* m_secondSubTiltleLabel;
    QVector<QPair<QString, QString>> m_basicenvironmentinfolist;
    QVector<QPair<QString, QString>> m_extrachoicesinfolist;
    QMap<QString, QVector<ComponentInfo>> m_componentInfomap;
    NcursesCheckBoxList* m_basicenvironmentlist;
    NcursesCheckBoxList* m_extrachoiceslist;
    bool m_isshow;
};

class ComponentFrame : public FrameInterface
{
    Q_OBJECT
public:
    ComponentFrame(FrameInterface* parent);
    virtual ~ComponentFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

private:
    void readConf();
    void writeConf();

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, InstallComponentFrame)
};

}
#endif // CLI_INSTALL_COMPONENT_FRAME_H
