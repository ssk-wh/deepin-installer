#ifndef DYNAMIC_DISK_WARNING_FRAME_H
#define DYNAMIC_DISK_WARNING_FRAME_H

#include "partman/device.h"

#include <QLabel>
#include <QWidget>
#include <utility>

class QHBoxLayout;
class QPushButton;

namespace installer {

class DynamicDiskWarningFrame : public QWidget {
    Q_OBJECT
public:
    explicit DynamicDiskWarningFrame(QWidget* parent = nullptr);
    void setDevice(const QList<Device::Ptr> list);
    void setWarningTip(const QString& tip);

signals:
    void requestCancel();
    void requestNext();

public slots:

protected:
    bool event(QEvent* event) override;

private:
    void refreshTs();

private:
    QLabel*      m_warning;
    QLabel*      m_warningTips;
    QPushButton*   m_cancelBtn;
    QPushButton*   m_acceptBtn;
    QHBoxLayout* m_diskListLayout;
};
}  // namespace installer

#endif  // !DYNAMIC_DISK_WARNING_FRAME_H
