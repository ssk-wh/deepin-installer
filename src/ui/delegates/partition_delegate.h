#pragma once

#include <QObject>

#include "partman/device.h"
#include "partman/operation.h"
#include "service/settings_manager.h"
#include "ui/delegates/advanced_validate_state.h"

namespace installer {
namespace partition {

class Delegate : public QObject {
    Q_OBJECT
public:
    explicit Delegate(QObject* parent = nullptr);

    // Get virtual device list.
    inline const DeviceList virtualDevices() const {
        return virtual_devices_;
    }

    // Select |partition| as root partition.
    // It may be separated into two partitions if needed.
    // Call validate() to check whether this |partition| is appropriate before
    // using it.
    virtual void selectPartition(const Partition::Ptr partition);

    inline const DeviceList selectedDevices() {
        return selected_devices;
    }

    inline const Partition::Ptr selectedPartition() {
        return selected_partition_;
    }

    // Get human readable operation descriptions.
    inline QStringList getOptDescriptions() const {
        QStringList descriptions;
        for (const Operation& operation : operations_) {
            descriptions.append(operation.description());
        }

        return descriptions;
    }

    // Get current operation list.
    inline OperationList operations() const {
        return operations_;
    }

    // Save real device list when it is refreshed.
    virtual void onDeviceRefreshed(const DeviceList& devices);

    // Set boot flag of root partition. Call this before operations() and after
    // validate().
    // Returns false if no appropriate partition can be set as bootable.
    virtual bool setBootFlag();

    // Get alternative partition type. Used while creating a new partition.
    // |partition| is an unallocated partition.
    virtual bool canAddLogical(const Partition::Ptr partition) const;
    virtual bool canAddPrimary(const Partition::Ptr partition) const;

    // Get physical device list.
    inline const DeviceList realDevices() const {
        return real_devices_;
    }

    // Returns true if current boot mode is mbr or any system is found on disks
    // with msdos partition table.
    virtual bool isMBRPreferred() const;

    // Check whether device at |device_path| is appropriate for current system.
    virtual bool isPartitionTableMatch(const QString& device_path) const;

    // Validate whether selected partition is appropriate.
    virtual ValidateStates validate() const;

    virtual bool createPartition(const Partition::Ptr partition,
                                 PartitionType        partition_type,
                                 bool                 align_start,
                                 FsType               fs_type,
                                 const QString&       mount_point,
                                 qint64               total_sectors,
                                 const QString&       label = QString());

    virtual bool createLogicalPartition(const Partition::Ptr partition,
                                        bool                 align_start,
                                        FsType               fs_type,
                                        const QString&       mount_point,
                                        qint64               total_sectors,
                                        const QString&       label = QString());

    virtual bool createPrimaryPartition(const Partition::Ptr partition,
                                        PartitionType        partition_type,
                                        bool                 align_start,
                                        FsType               fs_type,
                                        const QString&       mount_point,
                                        qint64               total_sectors,
                                        const QString&       label = QString());

    virtual Partition::Ptr deletePartition(const Partition::Ptr partition);

    virtual void formatPartition(const Partition::Ptr partition,
                                 FsType               fs_type,
                                 const QString&       mount_point);

    // Clear operation list.
    virtual void resetOperations();

    // Update bootloader settings to |path|.
    virtual void setBootloaderPath(const QString& path);

    virtual void onManualPartDone(const DeviceList &devices) = 0;

    inline const DiskPartitionSetting& settings() const {
        return settings_;
    }

    void refreshVisual();

    // Get real partition on disk where |virtual_partition| is located.
    Partition::Ptr getRealPartition(const Partition::Ptr virtual_partition) const;

signals:
    void deviceRefreshed(const DeviceList& devices);

protected:
    DeviceList     real_devices_;
    DeviceList     virtual_devices_;
    QString        bootloader_path_;
    OperationList  operations_;
    Partition::Ptr selected_partition_;
    int            primaryPartitionLength = 0;

    // device_path_list[0]:SystemDisk, [1]:DataDisk.
    QStringList selected_disks;
    DeviceList  selected_devices;

    DiskPartitionSetting settings_;
};
}  // namespace partition
}  // namespace installer
