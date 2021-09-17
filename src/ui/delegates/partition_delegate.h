#pragma once

#include <QObject>

#include "partman/device.h"
#include "partman/lvm_operation.h"
#include "service/settings_manager.h"
#include "ui/delegates/advanced_validate_state.h"

namespace installer {
namespace partition {

enum class PartitionAction {
    CreateLogicalPartition,
    RemoveLogicalPartition,
};

class Delegate : public QObject {
    Q_OBJECT
public:
    explicit Delegate(QObject* parent = nullptr);

    bool scanNvidia();

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
    virtual QStringList getOptDescriptions() const {
        QStringList descriptions;
        for (const Operation::Ptr operation : operations_) {
            descriptions.append(operation->description());
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

    virtual Partition* newPartition();

    virtual Partition* newPartition(const Partition &partition);

    virtual Operation* newOperation(const Device::Ptr device);

    virtual Operation* newOperation(OperationType type,
                                    const Partition::Ptr orig_partition,
                                    const Partition::Ptr new_partition);

    virtual bool createPartition(const Partition::Ptr partition,
                                 PartitionType        partition_type,
                                 bool                 align_start,
                                 FsType               fs_type,
                                 const QString&       mount_point,
                                 qint64               total_sectors,
                                 const QString&       label = QString(),
                                 bool                 isLvm = false,
                                 bool flag = true);

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
                                        const QString&       label = QString(),
                                        bool                 isLvm = false,
                                        bool flag = true);

    virtual void deletePartition(const Partition::Ptr partition);

    virtual void formatPartition(const Partition::Ptr partition,
                                 FsType               fs_type,
                                 const QString&       mount_point);

    void createDeviceTable(Device::Ptr device);

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

    // Clear mount point of operation.new_partition with value |mount_point|.
    void resetOperationMountPoint(const QString& mount_point);

    void updateMountPoint(const Partition::Ptr partition, const QString& mount_point);

    // make sure that extended partition exists here now before call it.
    // return true: success, start_sector & end_sector is new extended partition boundary.
    // return false: failed, start_sector & end_sector is unchanged.
    bool reCalculateExtPartBoundary(PartitionAction       action,
                                    const Partition::Ptr& current,
                                    qint64&               start_sector,
                                    qint64&               end_sector);
    bool reCalculateExtPartBoundary(const PartitionList&  partitions,
                                    PartitionAction       action,
                                    const Partition::Ptr& current,
                                    qint64&               start_sector,
                                    qint64&               end_sector);

    Device::Ptr findDevice(const QString& devicePath);   
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
