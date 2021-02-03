#ifndef CLI_INSTALL_PARTITION_FRAME_H
#define CLI_INSTALL_PARTITION_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"


namespace installer {

class NcursesLabel;
class NcursesListView;
class FullDiskFrame;
class AdvancedPartitionFrame;
//class PartitionFrame;
class PartitionModel;

enum ePartitionMode
{
    PAR_MOD_FULL_DISK = 0,
    PAR_MOD_MANUAL
};

class PartitionFramePrivate : public FrameInterfacePrivate
{
    //friend PartitionFrame;
    Q_OBJECT
public:
    PartitionFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {
        initUI();
        initConnection();
    }
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    virtual void onKeyPress(int keyCode) override;
    virtual void keyEventTriger(int keycode) override;
    ePartitionMode GetParMode(){ return m_parMode; }
    void setIsShow(bool isshow) override { m_isshow = isshow; }
    void setcurrentchoicetype(int type){ m_currentchoicetype = type; }
    void doBackBtnClicked();
    void doNextBtnClicked();
    void setValue();
signals:
    void showChildSignal(int index);
    void keyEventTrigerSignal(int keycode);
    void dostartInstall();
private:
    //NcursesLabel* m_label_title = nullptr;
    NcursesLabel* m_label_comment1 = nullptr;
    NcursesLabel* m_label_comment2 = nullptr;
    NcursesListView* m_partitionmodelist = nullptr;
    ePartitionMode m_parMode = PAR_MOD_FULL_DISK;
    QString m_localeString = "";
    bool m_isshow = false;
    int m_currentchoicetype = -1;//-1-main page 0-full disk page 1-advance page
    //int m_currentpageindex = -1;//
};

class PartitionFrame : public FrameInterface
{
    Q_OBJECT
public:
    PartitionFrame(FrameInterface* parent);
    virtual ~PartitionFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;
    void scanDevices();

private:
    void writeConf();

private slots:
    void showChildSlot(int index);
    //void FullDiskPartitionComplete();

protected:
    bool handle() override;
    PartitionModel* m_partition_model = nullptr;
    FullDiskFrame* m_fullDiskFrame = nullptr;
    AdvancedPartitionFrame* m_advancedPartitionFrame = nullptr;
    Q_DECLARE_PRIVATE_D(m_private, PartitionFrame)
};


}


#endif
