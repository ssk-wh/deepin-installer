#ifndef FULL_DISK_ENCRYPT_FRAME_H
#define FULL_DISK_ENCRYPT_FRAME_H

#include "partman/device.h"
#include "ui/interfaces/frameinterface.h"
#include "service/settings_manager.h"

#include <QWidget>
#include <DLineEdit>
#include <DPasswordEdit>
#include <DIconButton>
#include <DSuggestButton>
#include <pwd.h>

DWIDGET_USE_NAMESPACE

class QVBoxLayout;
class QLabel;
class QCheckBox;
class QPushButton;

namespace installer {
class RoundedProgressBar;
class TitleLabel;
class SystemInfoTip;
class FullDiskPartitionWidget;
class FullDiskDelegate;
class SelectButton;

#define  FULL_DISK_DISK_MAX_COUNT (2)

struct FullDiskDiskInfo {
    Device::Ptr m_device;
    QLabel* m_diskLbl;
    QLabel* m_devicePathLbl;
    QLabel* m_deviceModelLbl;
    QLabel* m_deviceSizeLbl;
};

class Full_Disk_Encrypt_frame : public ChildFrameInterface
{
    Q_OBJECT
public:
    explicit Full_Disk_Encrypt_frame(FrameProxyInterface* frameProxyInterface
                                     , FullDiskDelegate * delegate, QWidget *parent = nullptr);

    void onShowDeviceInfomation();

    bool focusSwitch() override;
    bool doSpace() override;
    bool doSelect() override;
    bool directionKey(int keyvalue) override;

signals:
    void cancel();
    void encryptFinished();

public:
    QString passwd();
    void getFinalDiskResolution(FinalFullDiskResolution& resolution);

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void initConnections();
    void onNextBtnClicked();
    void onEncryptUpdated(bool checked);
    void updateText();
    void updateDiskInfo(int index);
    void updateDiskInfo();
    void setupCloseButton();
    void encryptEditOnFocus(bool ison);
    void encryptRepeatEditOnFocus(bool ison);

private:
    QString m_passwd;
    QVBoxLayout *m_layout;
    TitleLabel *m_frameLbl;
    QLabel *m_frameSubLbl;
    FullDiskDiskInfo  m_diskinfo[FULL_DISK_DISK_MAX_COUNT];
    QLabel *m_encryptLbl;
    QLabel *m_encryptCheckLbl;
    QFrame *m_encryptFrame;
    QFrame *m_encryptCheckFrame;
    DPasswordEdit *m_encryptEdit;
    DPasswordEdit *m_encryptRepeatEdit;
    SelectButton *m_cancelBtn;
    DSuggestButton *m_confirmBtn;
    SystemInfoTip *m_errTip;
    QList<DLineEdit*> m_editList;
    FullDiskPartitionWidget *m_diskPartitionWidget;
    FullDiskDelegate *m_diskPartitionDelegate;
    DIconButton* m_close_button = nullptr;

};
}

#endif // FULL_DISK_ENCRYPT_FRAME_H
