#ifndef INSTALL_FULL_DISK_FRAME_CLI_H
#define INSTALL_FULL_DISK_FRAME_CLI_H

#include "ui/interfaces_cli/frameinterface.h"
#include "ui/delegates/partition_util.h"

namespace installer {

class NcursesListView;
class NcursesLabel;
//class FullDiskFrame;
class PartitionModel;
class FullDiskDelegate;
class PrepareInstallFrame;

class FullDiskFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
    //friend FullDiskFrame;
public:
    FullDiskFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {
        initUI();
        initConnection();
        hide();
        //updateTs();
    }

    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    void setIsShow(bool isshow){ m_isshow = isshow; }
    void doBackBtnClicked();
    void doNextBtnClicked();
    void setSystemDiskList(QStringList &info);
    NcursesListView* getSystemDiskList(){ return m_systemdisklist; }
    void setDataDiskList(QStringList &info);
    NcursesListView* getDataDiskList(){ return m_datadisklist; }
    void showListView();

signals:
    void backToPreviousPage();
    void allIsFinished();
    void keyEventTrigerSignal(int keycode);
    void startInstall();
private slots:
    void keyPresseEvent(int keycode);

private:
    NcursesLabel* m_label_title       = nullptr;
    NcursesLabel* m_label_systemdisk  = nullptr;
    NcursesListView* m_systemdisklist = nullptr;
    NcursesLabel* m_label_datadisk    = nullptr;
    NcursesListView* m_datadisklist   = nullptr;
    bool m_isshow = false;
    //DeviceList m_devices;
    int m_currentchoicetype = -1;
};

class FullDiskFrame : public FrameInterface
{
    Q_OBJECT
public:
    FullDiskFrame(FrameInterface* parent, PartitionModel* model);
    virtual ~FullDiskFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;
    bool doFullDiskPartition();
    void onManualPartDone(bool ok, const DeviceList& devices);

public slots:
    void onDeviceRefreshed(const DeviceList& devices);

private:
    void readConf();
    void writeConf();

protected:
    bool handle() override;
private:
    DeviceList m_DeviceList;
    FullDiskDelegate* m_delegate;
    PartitionModel* m_partitionModel;
    PrepareInstallFrame* m_prepareInstallFrame;
    Q_DECLARE_PRIVATE_D(m_private, FullDiskFrame)
};


}


#endif
