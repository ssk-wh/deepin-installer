#ifndef FULL_DISK_ENCRYPT_FRAME_H
#define FULL_DISK_ENCRYPT_FRAME_H

#include "partman/device.h"

#include <QWidget>

class QVBoxLayout;
class QLabel;
class QCheckBox;

namespace installer {
class RoundedProgressBar;
class NavButton;
class LineEdit;
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

    void setDevice(Device::Ptr device);

signals:
    void cancel();
    void finished();

protected:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private:
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
    FullDiskDiskInfo  m_diskinfo[FULL_DISK_DISK_MAX_COUNT];
    QCheckBox *m_encryptCheck;
    QLabel *m_encryptLbl;
    QLabel *m_encryptCheckLbl;
    LineEdit *m_encryptEdit;
    LineEdit *m_encryptRepeatEdit;
    NavButton *m_cancelBtn;
    NavButton *m_nextBtn;
    SystemInfoTip *m_errTip;
    QList<LineEdit*> m_editList;
    FullDiskPartitionWidget *m_diskPartitionWidget;
    FullDiskDelegate *m_diskPartitionDelegate;

};
}

#endif // FULL_DISK_ENCRYPT_FRAME_H
