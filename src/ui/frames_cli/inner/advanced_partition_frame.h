#ifndef INSTALL_ADVANCED_PARTITION_FRAME_CLI_H
#define INSTALL_ADVANCED_PARTITION_FRAME_CLI_H

#include "ui/interfaces_cli/frameinterface.h"
#include "ui/delegates/partition_util.h"

namespace installer {

class NcursesLabel;
class NcursesListView;
class PartitionModel;
class AdvancedPartitionDelegate;
class ValidateState;
typedef QList <ValidateState> ValidateStates;
class LvmPartitionDelegate;
//class AdvancedPartitionFrame;
class AdvancedPartitionFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
    //friend AdvancedPartitionFrame;
public:
    AdvancedPartitionFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {
        initUI();
        initConnection();
//        updateTs();
    }

    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    void setIsShow(bool isshow){ m_isshow = isshow; }
    void onDeviceRefreshed(const DeviceList& devices);
    void setMsgHeadLable(QString& msg);
    void setErrorLable(QStringList& error);
    void onKeyPress(int keycode) override;
    void doBackBtnClicked();
    void doNextBtnClicked();
    void setchildFoursEnabel(bool enabel);
    void setCurrentchoicetype(int state);
    Partition::Ptr getCurrentPartition();
signals:
    void newPartition(const Partition::Ptr& partition);
    void deletePartition(const Partition::Ptr& partition);
    void keyEventTrigerSignal(int keycode);
    void backToPreviousPage();
    void allIsFinished();
    void doBackBtnClickedSignal();
    void doNectBtnClickedSignal();
    void startInstall();

private slots:
    void keyPresseEvent(int keycode);

private:
    NcursesLabel* m_label_title = nullptr;
    NcursesLabel* m_label_tips = nullptr;
    NcursesListView* m_listViewPartitionMode = nullptr;
    NcursesListView* m_errorLabel = nullptr;
    NcursesLabel* m_msgHeadLabel = nullptr;
    bool m_isshow = false;
    DeviceList m_devices;
    int m_currentchoicetype = -1;
};

class AdvancedPartitionFrame : public FrameInterface
{
    Q_OBJECT
public:
    AdvancedPartitionFrame(FrameInterface* parent, PartitionModel* model);
    virtual ~AdvancedPartitionFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;
    void clearErrorMessages();
    bool validate();
    // Returns error message related to this |state|.
    QString validateStateToText(ValidateState state);

    void showErrorMessages();

    // Update header text based on current states.
    void updateErrorMessageHeader();

    // Update error messages and validate states.
    void updateValidateStates();

    void setShowEnable( bool isShow);

    // Emitted when partition table not appropriate at |device_path|.
    void requestNewTable(const QString& device_path);

    // Show PartitionNumberLimitationFrame.
    void requestPartitionNumberLimitationFrame();
public slots:
    void onDeviceRefreshed(const DeviceList& devices);
    void onNewPartition(const Partition::Ptr& partition);
    void onDeletePartitionTriggered(const Partition::Ptr partition);
    void doBackBtnClicked();
    void doNextBtnClicked();
    void onPrepareInstallFrameFinished();
    void onManualPartDone(bool ok, const DeviceList& devices);
private:
    void readConf();
    void writeConf();
protected:
    bool handle() override;
private:
    Q_DECLARE_PRIVATE_D(m_private, AdvancedPartitionFrame)
    PartitionModel* m_partitionModel;
    AdvancedPartitionDelegate* m_delegate = nullptr;
    LvmPartitionDelegate* m_lvmDelegate = nullptr;
    AdvancedPartitionDelegate* m_currentDelegate = nullptr;
    ValidateStates m_validateStates;
    bool m_isShow = false;
};



}

#endif
