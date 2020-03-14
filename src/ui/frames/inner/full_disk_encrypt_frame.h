#ifndef FULL_DISK_ENCRYPT_FRAME_H
#define FULL_DISK_ENCRYPT_FRAME_H

#include "partman/device.h"

#include <QWidget>
#include <DLineEdit>
#include <DPasswordEdit>

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

#define  FULL_DISK_DISK_MAX_COUNT (2)

struct FullDiskDiskInfo {
    Device::Ptr m_device;
    QLabel* m_diskLbl;
    QLabel* m_devicePathLbl;
    QLabel* m_deviceModelLbl;
    QLabel* m_deviceSizeLbl;
};

class Full_Disk_Encrypt_frame : public QWidget
{
    Q_OBJECT
public:
    explicit Full_Disk_Encrypt_frame(FullDiskDelegate * delegate, QWidget *parent = nullptr);

    void onShowDeviceInfomation();

signals:
    void cancel();
    void finished();

protected:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private:
    void initConnections();
    void onNextBtnClicked();
    void onEncryptUpdated(bool checked);
    void updateText();
    void updateEditCapsLockState(bool on);
    void updateDiskInfo(int index);
    void updateDiskInfo();

private:
    QVBoxLayout *m_layout;
    TitleLabel *m_frameLbl;
    QLabel *m_frameSubLbl;
    QLabel *m_tilabel;
    FullDiskDiskInfo  m_diskinfo[FULL_DISK_DISK_MAX_COUNT];
    QLabel *m_encryptLbl;
    QLabel *m_encryptCheckLbl;
    DPasswordEdit *m_encryptEdit;
    DPasswordEdit *m_encryptRepeatEdit;
    QPushButton *m_cancelBtn;
    QPushButton *m_nextBtn;
    SystemInfoTip *m_errTip;
    QList<DLineEdit*> m_editList;
    FullDiskPartitionWidget *m_diskPartitionWidget;
    FullDiskDelegate *m_diskPartitionDelegate;

};
}

#endif // FULL_DISK_ENCRYPT_FRAME_H
