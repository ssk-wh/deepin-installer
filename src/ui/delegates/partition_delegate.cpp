#include "partition_delegate.h"

#include "partman/partition.h"
#include "service/settings_name.h"
#include "ui/delegates/partition_util.h"
#include "base/command.h"

#include <QUuid>

using namespace installer;
using namespace installer::partition;

Delegate::Delegate(QObject* parent) : QObject(parent) {}

bool Delegate::scanNvidia()
{
    if (!GetSettingsBool(KEnableInstallNvidiaDriver) || isNotebook()) {
        return false;
    }

    QString cmd("lspci");
    QStringList args("-n");
    QString output;
    if (!SpawnCmd(cmd, args, output)) {
       return false;
    }

    return (output.indexOf(QRegExp(".*03(80|0[0-2]): 10de")) > -1);
}

bool Delegate::canAddLogical(const Partition::Ptr partition) const
{
    const int index = DeviceIndex(virtual_devices_, partition->device_path);
    if (index == -1) {
        qCritical() << "getSupportedPartitionType() no device found at:"
                    << partition->device_path;
        return false;
    }

    const Device::Ptr device = virtual_devices_.at(index);

    // If partition table is empty, always returns false.
    // Thus, at least one primary partition shall be created.
    if (device->table == PartitionTableType::Empty) {
        return false;
    }

    // Ignores gpt table.
    if (device->table != PartitionTableType::MsDos) {
        return false;
    }

    bool      logical_ok = true;
    const int ext_index  = ExtendedPartitionIndex(device->partitions);
    if (ext_index == -1) {
        // No extended partition found, so check a new primary partition is
        // available or not.
        if (GetPrimaryPartitions(device->partitions).length() >= device->max_prims) {
            logical_ok = false;
        }
    }
    else {
        // Check whether there is primary partition between |partition| and
        // extended partition->
        const Partition::Ptr ext_partition   = device->partitions.at(ext_index);
        const PartitionList  prim_partitions = GetPrimaryPartitions(device->partitions);
        if (partition->end_sector < ext_partition->start_sector) {
            for (Partition::Ptr prim_partition : prim_partitions) {
                if (prim_partition->end_sector > partition->start_sector &&
                    prim_partition->start_sector < ext_partition->start_sector) {
                    logical_ok = false;
                }
            }
        }
        else if (partition->start_sector > ext_partition->end_sector) {
            for (Partition::Ptr prim_partition : prim_partitions) {
                if (prim_partition->end_sector < partition->start_sector &&
                    prim_partition->start_sector > ext_partition->end_sector) {
                    logical_ok = false;
                }
            }
        }
    }
    return logical_ok;
}

bool Delegate::canAddPrimary(const Partition::Ptr partition) const
{
    const int index = DeviceIndex(virtual_devices_, partition->device_path);
    if (index == -1) {
        qCritical() << "getSupportedPartitionType() no device found at:"
                    << partition->device_path;
        return false;
    }

    const Device::Ptr device = virtual_devices_.at(index);

    // If partition table is empty, always returns true.
    if (device->table == PartitionTableType::Empty) {
        return true;
    }

    const PartitionList prim_partitions    = GetPrimaryPartitions(device->partitions);
    const PartitionList logical_partitions = GetLogicalPartitions(device->partitions);

    // Check primary type
    bool primary_ok = true;
    if (prim_partitions.length() >= device->max_prims) {
        primary_ok = false;
    }
    else {
        // Check whether |partition| is between two logical partitions.
        bool has_logical_before = false;
        bool has_logical_after  = false;
        for (Partition::Ptr logical_partition : logical_partitions) {
            if (logical_partition->start_sector < partition->start_sector) {
                has_logical_before = true;
            }
            if (logical_partition->end_sector > partition->end_sector) {
                has_logical_after = true;
            }
        }
        if (has_logical_after && has_logical_before) {
            primary_ok = false;
        }
    }

    return primary_ok;
}

bool Delegate::isMBRPreferred() const
{
    return IsMBRPreferred(real_devices_);
}

bool Delegate::isPartitionTableMatch(const QString& device_path) const
{
    return IsPartitionTableMatch(real_devices_, device_path);
}

void Delegate::selectPartition(const Partition::Ptr partition)
{
    selected_partition_ = partition;
}

bool Delegate::setBootFlag()
{
    bool found_boot = false;

    // First check new EFI partition->
    for (Operation::Ptr operation : operations_) {
        if (operation->type == OperationType::NewPartTable) continue;
        if (operation->new_partition->fs == FsType::EFI) {
            operation->new_partition->flags.append(PartitionFlag::Boot);
            operation->new_partition->flags.append(PartitionFlag::ESP);
            found_boot = true;
        }
    }

    // Check existing EFI partition->
    for (Device::Ptr device : virtual_devices_) {
        for (Partition::Ptr partition : device->partitions) {
            if (partition->fs == FsType::EFI) {
                return true;
            }
        }
    }

    // Check /boot partition->
    if (!found_boot) {
        for (Operation::Ptr operation : operations_) {
            if (operation->type == OperationType::NewPartTable) continue;
            if (operation->new_partition->mount_point == kMountPointBoot) {
                operation->new_partition->flags.append(PartitionFlag::Boot);
                found_boot = true;
            }
        }
    }

    // At last, check / partition->
    if (!found_boot) {
        for (Operation::Ptr operation : operations_) {
            if (operation->type == OperationType::NewPartTable) continue;
            if (operation->new_partition->mount_point == kMountPointRoot) {
                operation->new_partition->flags.append(PartitionFlag::Boot);
                found_boot = true;
            }
        }
    }

    return found_boot;
}

ValidateStates Delegate::validate() const
{
    // Policy:
    // * Returns ok if partition table of selected partition is empty.
    // * Check / partition is set.
    // * Check / partition is large enough.
    const Partition::Ptr root_partition = selected_partition_;
    ValidateStates       states;

    // Check partition table is empty or not.
    const int device_index = DeviceIndex(virtual_devices_, root_partition->device_path);
    if (device_index == -1) {
        qCritical() << "validate() device index out of range:"
                    << root_partition->device_path;
        states << ValidateState::RootMissing;
    }

    const Device::Ptr device = virtual_devices_.at(device_index);

    if (device->table == PartitionTableType::Empty) {
        states << ValidateState::Ok;
    }

    if (root_partition->device_path.isEmpty()) {
        states << ValidateState::RootMissing;
    }

    // If currently selected device reaches its maximum primary partitions and
    // root partition is a Freespace, it is impossible to created a new
    // primary partition->
    if ((root_partition->type == PartitionType::Unallocated) &&
        !this->canAddPrimary(root_partition) && !this->canAddLogical(root_partition)) {
        states << ValidateState::MaxPrimPartErr;
    }

    const int    root_required      = GetSettingsInt(kPartitionRootMiniSpace);//
    const qint64 root_minimum_bytes = root_required * kGibiByte;
    const qint64 root_real_bytes    = root_partition->getByteLength() + kMebiByte;
    if (root_real_bytes < root_minimum_bytes) {
        states << ValidateState::RootTooSmall;
    }

    if (states.isEmpty()) {
        states << ValidateState::Ok;
    }

    return states;
}

void Delegate::createDeviceTable(Device::Ptr device) {
    // Add NewPartTable operation.
    Device::Ptr new_device(new Device(*device));
    new_device->partitions.clear();
    new_device->table =
        IsEfiEnabled() ? PartitionTableType::GPT : PartitionTableType::MsDos;
    Operation::Ptr operation(newOperation(new_device));
    operations_.append(operation);
    // Update virtual device property at the same time.
    operation->applyToVisual(device);
}

void Delegate::resetOperations()
{
    operations_.clear();
    virtual_devices_       = FilterInstallerDevice(real_devices_);
    primaryPartitionLength = 0;
}

Partition* Delegate::newPartition() {
    return new Partition;
}

Partition* Delegate::newPartition(const Partition &partition) {
    return new Partition(partition);
}

Operation* Delegate::newOperation(const Device::Ptr device) {
    return new Operation(device);
}

Operation* Delegate::newOperation(OperationType type,
                                const Partition::Ptr orig_partition,
                                  const Partition::Ptr new_partition) {
    return new Operation(type, orig_partition, new_partition);
}

bool Delegate::createPartition(const Partition::Ptr partition,
                               PartitionType        partition_type,
                               bool                 align_start,
                               FsType               fs_type,
                               const QString&       mount_point,
                               qint64               total_sectors,
                               const QString&       label,
                               bool                 isLvm,
                               bool flag)
{
    Device::Ptr device = findDevice(partition->device_path);

    if (device.isNull()) {
        return false;
    }

    // * If partition table is empty, create a new one.
    if (device->table == PartitionTableType::Empty) {
        createDeviceTable(device);
            //NOTE: GPT table need 33 sectors in the end.
        if (operations_.last()->device->table == PartitionTableType::GPT) {
            partition->length -= 33;
            partition->end_sector -= 33;
        }
    }

    if (partition_type == PartitionType::Normal) {
        return createPrimaryPartition(partition, partition_type, align_start, fs_type,
                                      mount_point, total_sectors, label, isLvm, flag);
    }
    else if (partition_type == PartitionType::Logical) {
        return createLogicalPartition(partition, align_start, fs_type, mount_point,
                                      total_sectors, label);
    }
    else {
        qCritical() << "not supported partition type:" << partition_type;
        return false;
    }
}

bool Delegate::createLogicalPartition(const Partition::Ptr partition,
                                      bool                 align_start,
                                      FsType               fs_type,
                                      const QString&       mount_point,
                                      qint64               total_sectors,
                                      const QString&       label)
{
    // Policy:
    // * Create extended partition if not found;
    // * If new logical partition is not contained in or is intersected with
    //   extended partition, enlarge extended partition
    Device::Ptr device = findDevice(partition->device_path);

    if (device.isNull()) {
        return false;
    }

    int               ext_index = ExtendedPartitionIndex(device->partitions);
    Partition::Ptr    ext_partition(newPartition());

    if (ext_index == -1) {
        // TODO(xushaohua): Support extended partition in simple mode.
        qCritical() << "Cannot create extended partition in simple mode";
        //    return false;

        // No extended partition found, create one.
        if (!createPrimaryPartition(partition, PartitionType::Extended, align_start,
                                    FsType::Empty, "", total_sectors)) {
            qCritical() << "Failed to create extended partition";
            return false;
        }

        ext_partition = operations_.last()->new_partition;
    }
    else {
        // No need to add extended partition or enlarge it.
        ext_partition = device->partitions.at(ext_index);

        // Enlarge extended partition if needed.
        if (ext_partition->start_sector > partition->start_sector ||
            ext_partition->end_sector < partition->end_sector) {
            Partition::Ptr new_ext_partition(newPartition(*ext_partition));
            new_ext_partition->start_sector =
                qMin(ext_partition->start_sector, partition->start_sector);
            new_ext_partition->end_sector =
                qMax(ext_partition->end_sector, partition->end_sector);

            AlignPartition(new_ext_partition);

            Operation::Ptr resize_ext_operation(newOperation(OperationType::Resize, ext_partition,
                                                       new_ext_partition));
            resize_ext_operation->device = device;
            ext_partition = new_ext_partition;
            operations_.append(resize_ext_operation);
            resize_ext_operation->applyToVisual(device);
        }
    }

    Partition::Ptr new_partition(newPartition());
    new_partition->device_path = partition->device_path;
    new_partition->path        = partition->path;
    new_partition->sector_size = partition->sector_size;
    new_partition->status      = PartitionStatus::New;
    new_partition->type        = PartitionType::Logical;
    new_partition->fs          = fs_type;
    new_partition->mount_point = mount_point;
    new_partition->label       = label;

    const int partition_number = AllocLogicalPartitionNumber(device);
    if (partition_number < 0) {
        qCritical() << "Failed to allocate logical part number!";
        return false;
    }

    new_partition->changeNumber(partition_number);

    if (fs_type == FsType::Recovery) {
        // Hide recovery partition
        new_partition->flags << PartitionFlag::Hidden;
        new_partition->label       = "Backup";
        new_partition->mount_point = "/recovery";
    }

    // space is required for the Extended Boot Record.
    // Generally an additional track or MebiByte is required so for
    // our purposes reserve a MebiByte in front of the partition
    if (align_start) {
        // Align from start of |partition|.
        // Add space for Extended Boot Record.
        const qint64 start_sector =
            qMax(partition->start_sector, ext_partition->start_sector);
        new_partition->start_sector = start_sector;

        const qint64 end_sector = qMin(partition->end_sector, ext_partition->end_sector);
        new_partition->end_sector =
            qMin(end_sector, total_sectors + new_partition->start_sector);
    }
    else {
        new_partition->end_sector =
            qMin(partition->end_sector, ext_partition->end_sector);
        const qint64 start_sector =
            qMax(partition->start_sector, ext_partition->start_sector);
        new_partition->start_sector = qMax(start_sector,
                                           partition->end_sector - total_sectors);
    }

    new_partition->start_sector += 1;

    // Align to nearest MebiBytes.
    AlignPartition(new_partition);

    // Check partition sector range.
    // Also check whether partition size is less than 1MiB or not.
    if (new_partition->start_sector < partition->start_sector ||
        new_partition->start_sector >= partition->end_sector ||
        new_partition->getByteLength() < kMebiByte ||
        new_partition->end_sector > partition->end_sector) {
        qCritical() << "Invalid partition sector range";
        return false;
    }

    resetOperationMountPoint(mount_point);
    Operation::Ptr operation(newOperation(OperationType::Create, partition, new_partition));
    operation->device = device;
    operations_.append(operation);
    operation->applyToVisual(device);

    return true;
}

bool Delegate::createPrimaryPartition(const Partition::Ptr partition,
                                      PartitionType        partition_type,
                                      bool                 align_start,
                                      FsType               fs_type,
                                      const QString&       mount_point,
                                      qint64               total_sectors,
                                      const QString&       label,
                                      bool                 isLvm,
                                      bool flag)
{
    // Policy:
    // * If new primary partition is contained in or intersected with
    //   extended partition, shrink extended partition or delete it if no other
    //   logical partitions.
    if (partition_type != PartitionType::Normal &&
        partition_type != PartitionType::Extended) {
        qCritical() << "createPrimaryPartition() invalid part type:" << partition_type;
        return false;
    }

    Device::Ptr device = findDevice(partition->device_path);

    if (device.isNull()) {
        return false;
    }

    const qint64 oneMebiByteSector = 1 * kMebiByte / partition->sector_size;

    // Shrink extended partition if needed.
    const int ext_index = ExtendedPartitionIndex(device->partitions);
    if (partition_type == PartitionType::Normal && ext_index > -1) {
        const Partition::Ptr ext_partition = device->partitions.at(ext_index);
        const PartitionList  logical_parts = GetLogicalPartitions(device->partitions);
        if (logical_parts.isEmpty()) {
            // Remove extended partition if no logical partitions.
            Partition::Ptr unallocated_partition(newPartition());
            unallocated_partition->device_path = ext_partition->device_path;
            // Extended partition does not contain any sectors.
            // This new allocated partition will be merged to other unallocated
            // partitions.
            unallocated_partition->start_sector = ext_partition->start_sector;
            unallocated_partition->end_sector   = ext_partition->end_sector;
            unallocated_partition->sector_size  = ext_partition->sector_size;
            unallocated_partition->type         = PartitionType::Unallocated;

            Operation::Ptr operation(newOperation(OperationType::Delete, ext_partition,
                                                       unallocated_partition));
            operations_.append(operation);
            operation->applyToVisual(device);

            // Remove extended partition from partition list explicitly.
            device->partitions.removeAt(ext_index);
        }
        else if (IsPartitionsJoint(ext_partition, partition)) {
            // Shrink extended partition to fit logical partitions.
            Partition::Ptr new_ext_part(newPartition(*ext_partition));
            new_ext_part->start_sector = logical_parts.first()->start_sector - oneMebiByteSector;
            new_ext_part->end_sector   = logical_parts.last()->end_sector;

            if (IsPartitionsJoint(new_ext_part, partition)) {
                qCritical() << "Failed to shrink extended partition!";
                return false;
            }

            Operation::Ptr operation(newOperation(OperationType::Resize, ext_partition, new_ext_part));
            operations_.append(operation);
            operation->applyToVisual(device);
        }
    }

    Partition::Ptr new_partition(newPartition());
    new_partition->device_path = partition->device_path;
    new_partition->path        = partition->path;
    new_partition->sector_size = partition->sector_size;
    new_partition->status      = PartitionStatus::New;
    new_partition->type        = partition_type;
    new_partition->fs          = fs_type;
    new_partition->mount_point = mount_point;
    new_partition->label       = label;
    new_partition->is_lvm      = isLvm;

    if (device->device_type == DeviceType::VGDevice || !new_partition->is_lvm) {
        int partition_number = AllocPrimaryPartitionNumber(device);
        if (partition_number < 0) {
            qCritical() << "Failed to allocate primary partition number!";
            return false;
        }
        new_partition->changeNumber(partition_number);
    }

    if (fs_type == FsType::Recovery) {
        // Hide recovery partition
        new_partition->flags << PartitionFlag::Hidden;
        new_partition->label       = "Backup";
        new_partition->mount_point = "/recovery";
    }

    // Check whether space is required for the Master Boot Record.
    // Generally an additional track or MebiByte is required so for
    // our purposes reserve a MebiByte in front of the partition->
    const bool need_mbr = (partition->start_sector <= oneMebiByteSector);
    if (align_start) {
        // Align from start of |partition|.
        if (need_mbr) {
            new_partition->start_sector = oneMebiByteSector;
        }
        else {
            new_partition->start_sector = partition->start_sector;
        }
        // Align to nearest MebiBytes.        
        if (!flag && (mount_point != kMountPointBoot) && (fs_type != FsType::EFI)) AlignPartition(new_partition);

        new_partition->end_sector =
            qMin(partition->end_sector, total_sectors + new_partition->start_sector);
    }
    else {
        new_partition->end_sector = partition->end_sector;
        if (need_mbr) {
            new_partition->start_sector =
                qMax(oneMebiByteSector, partition->end_sector - total_sectors);
        }
        else {
            new_partition->start_sector =
                qMax(partition->start_sector, partition->end_sector - total_sectors);
        }

        if (!flag && (mount_point != kMountPointBoot) && (fs_type != FsType::EFI)) AlignPartition(new_partition);
    }

    // Check partition sector range.
    // Also check whether partition size is less than 1MiB or not.
    if (new_partition->start_sector < partition->start_sector ||
        new_partition->start_sector >= partition->end_sector ||
        new_partition->getByteLength() < kMebiByte ||
        new_partition->end_sector > partition->end_sector) {
        qCritical() << "Invalid partition sector range"
                    << ", new_partition:" << new_partition << ", partition:" << partition;
        return false;
    }

    resetOperationMountPoint(mount_point);
    Operation::Ptr operation(newOperation(OperationType::Create, partition, new_partition));
    operation->device = device;
    operations_.append(operation);
    operation->applyToVisual(device);

    primaryPartitionLength++;

    return true;
}

void Delegate::deletePartition(const Partition::Ptr partition)
{
    // Policy:
    //  * Remove selected partition->
    //  * Merge unallocated partitions.
    //  * Remove extended partition if no logical partitions found.
    //  * Update partition number if needed.

    Partition::Ptr new_partition(newPartition(*partition));
    new_partition->partition_number = -1;
    new_partition->device_path      = partition->device_path;
    new_partition->sector_size      = partition->sector_size;
    new_partition->start_sector     = partition->start_sector;
    new_partition->end_sector       = partition->end_sector;
    new_partition->type             = PartitionType::Unallocated;
    new_partition->fs               = FsType::Empty;
    new_partition->status           = PartitionStatus::Delete;
    new_partition->mount_point      = "";

    //1MB space before logical partition must be recycled.
    if (partition->type == PartitionType::Logical) {
        const qint64 oneMebiByteSector = 1 * kMebiByte / partition->sector_size;
        new_partition->start_sector -= oneMebiByteSector;
    }

    Device::Ptr device = findDevice(partition->device_path);

    if (device.isNull()) {
        return;
    }

//TODO: Fix this bug using a pretty method!
#ifdef sadhu
    if (partition->status == PartitionStatus::New) {
        // If status of old partition is New, there shall be a CreateOperation
        // which generates that partition-> Merge that CreateOperation
        // with DeleteOperation.

        // TODO(xushaohua): Move to operation.h
        for (int index = operations_.length() - 1; index >= 0; --index) {
            const Operation::Ptr operation = operations_.at(index);
            if (operation->type == OperationType::Create &&
                *operation->new_partition.data() == *partition.data()) {
                partition->type        = PartitionType::Unallocated;
                partition->fs          = FsType::Empty;
                partition->status      = PartitionStatus::Delete;
                partition->mount_point = "";

                qDebug() << "delete partition info: " << *partition.data();

                const qint64 start_size = operation->orig_partition->start_sector;

                operations_.removeAt(index);

                // 修改操作，把相邻分区的operation中的orig partition向前补齐
                const qint64 end_size = partition->end_sector + 1;
                for (auto it = operations_.begin(); it != operations_.end(); ++it) {
                    if (it[0]->type == OperationType::Create &&
                        partition->device_path == it[0]->orig_partition->device_path &&
                        it[0]->orig_partition->start_sector == end_size) {
                        it[0]->orig_partition->start_sector = start_size;
                    }
                }
                break;
            }
        }
    }
    else {
#endif
        Operation::Ptr operation(newOperation(OperationType::Delete, partition, new_partition));
        operations_.append(operation);
        operation->device = device;
        operation->applyToVisual(device);
        qDebug() << "add delete operation" << *new_partition.data();
#ifdef sadhu
    }
#endif

    if (partition->type == PartitionType::Logical) {
        // Delete extended partition if needed.
        PartitionList& partitions = device->partitions;

        Partition::Ptr extPartition = partitions.at(ExtendedPartitionIndex(partitions));
        const PartitionList logical_parts = GetLogicalPartitions(partitions);
        if (!extPartition.isNull()) {
            // Only one logical partition, and it has been removed.
            // Or logical partition list is empty.
            if ((logical_parts.length() == 1 && logical_parts.at(0) == partition) ||
                (logical_parts.length() == 0)) {
                Partition::Ptr       unallocated_partition(newPartition());
                unallocated_partition->device_path = extPartition->device_path;
                // Extended partition does not contain any sectors.
                // This new allocated partition will be merged to other unallocated
                // partitions.
                unallocated_partition->start_sector = extPartition->start_sector;
                unallocated_partition->end_sector   = extPartition->end_sector;
                unallocated_partition->sector_size  = extPartition->sector_size;
                unallocated_partition->type         = PartitionType::Unallocated;
                Operation::Ptr operation(newOperation(OperationType::Delete, extPartition,
                                          unallocated_partition));
                operations_.append(operation);
                operation->applyToVisual(device);
            }
            else {
                //Modify boundary of extended partition after removing logical partition since it may be the first or the last logical partition
                qint64 ext_start_sector = -1;
                qint64 ext_end_sector   = -1;
                if (reCalculateExtPartBoundary(
                        partitions, PartitionAction::RemoveLogicalPartition, partition,
                        ext_start_sector, ext_end_sector)) {
                    Partition::Ptr       new_ext_partition(newPartition(*extPartition));
                    new_ext_partition->start_sector = ext_start_sector;
                    new_ext_partition->end_sector   = ext_end_sector;
                    Operation::Ptr operation(newOperation(OperationType::Resize, extPartition,
                                              new_ext_partition));
                    operations_.append(operation);
                    operation->applyToVisual(device);
                }
            }
        }
    }

    // TODO(xushaohua): Update partition number.
}

void Delegate::formatPartition(const Partition::Ptr partition,
                               FsType               fs_type,
                               const QString&       mount_point)
{
    qDebug() << "formatSimplePartition()" << partition << fs_type << mount_point;

    resetOperationMountPoint(mount_point);

    // Update partition of old operation, instead of adding a new one.
    // TODO(xushaohua): Move to operation.h
    if (partition->status == PartitionStatus::New ||
        partition->status == PartitionStatus::Format) {
        for (int index = operations_.length() - 1; index >= 0; --index) {
            Operation::Ptr operation = operations_[index];
            if ((operation->new_partition->path == partition->path) &&
                (operation->type == OperationType::Format ||
                 operation->type == OperationType::Create)) {
                operation->new_partition->mount_point = mount_point;
                operation->new_partition->fs          = fs_type;
                return;
            }
        }
    }

    Partition::Ptr new_partition(new Partition);
    new_partition->sector_size      = partition->sector_size;
    new_partition->start_sector     = partition->start_sector;
    new_partition->end_sector       = partition->end_sector;
    new_partition->path             = partition->path;
    new_partition->device_path      = partition->device_path;
    new_partition->partition_number = partition->partition_number;
    new_partition->fs               = fs_type;
    new_partition->type             = partition->type;
    new_partition->mount_point      = mount_point;

    if (partition->status == PartitionStatus::Real) {
        new_partition->status = PartitionStatus::Format;
    }
    else if (partition->status == PartitionStatus::New ||
             partition->status == PartitionStatus::Format) {
        new_partition->status = partition->status;
    }

    if (fs_type == FsType::Recovery) {
        // Hide recovery partition
        new_partition->flags << PartitionFlag::Hidden;
        new_partition->label       = "Backup";
        new_partition->mount_point = "/recovery";
    }

    Device::Ptr device = findDevice(partition->device_path);

    if (device.isNull()) {
        return;
    }

    Operation::Ptr operation(newOperation(OperationType::Format, partition, new_partition));
    operations_.append(operation);
    operation->applyToVisual(device);
}

void Delegate::onDeviceRefreshed(const DeviceList& devices)
{
    real_devices_ = devices;
    operations_.clear();
    virtual_devices_ = FilterInstallerDevice(real_devices_);

    for (Device::Ptr device : virtual_devices_) {
        device->partitions = FilterFragmentationPartition(device->partitions);
    }

    emit deviceRefreshed(virtual_devices_);
}

void Delegate::setBootloaderPath(const QString& path)
{
    bootloader_path_ = path;
}

void Delegate::refreshVisual()
{
    // Filters partition list based on the following policy:
    // * Remove extended partition if no logical partition exists;
    // * Merge unallocated partition with next unallocated one;
    // * Ignore partitions with size less than 100Mib;

    virtual_devices_ = FilterInstallerDevice(real_devices_);

    for (Device::Ptr device : virtual_devices_) {
        device->partitions = FilterFragmentationPartition(device->partitions);
    }

    for (Device::Ptr device : virtual_devices_) {
        // Merge unallocated partitions.
        MergeUnallocatedPartitions(device->partitions);

        for (Operation::Ptr operation : operations_) {
            if ((operation->type == OperationType::NewPartTable &&
                 *operation->device.data() == *device.data()) ||
                (operation->type != OperationType::NewPartTable &&
                 operation->orig_partition->device_path == device->path)) {
                operation->applyToVisual(device);
            }
        }

        // Merge unallocated partitions.
        MergeUnallocatedPartitions(device->partitions);
    }

    qDebug() << "devices:" << virtual_devices_;
    qDebug() << "operations:" << operations_;
    emit deviceRefreshed(virtual_devices_);
}

Partition::Ptr Delegate::getRealPartition(const Partition::Ptr virtual_partition) const
{
    const int index = DeviceIndex(real_devices_, virtual_partition->device_path);
    if (index == -1) {
        qWarning() << "failed to find device:" << virtual_partition->device_path;
        return Partition::Ptr();
    }

    for (Partition::Ptr partition : real_devices_.at(index)->partitions) {
        // Ignores extended partition->
        if (partition->type == PartitionType::Extended) {
            continue;
        }
        if ((partition->start_sector <= virtual_partition->start_sector) &&
            (partition->end_sector >= virtual_partition->end_sector)) {
            return partition;
        }
    }

    qWarning() << "Failed to find partition at:" << virtual_partition;
    return Partition::Ptr();
}

void Delegate::resetOperationMountPoint(const QString& mount_point) {
    qDebug() << Q_FUNC_INFO << mount_point;

    for (auto it = operations_.begin(); it != operations_.end(); ++it) {
        Operation::Ptr operation = *it;
        if (operation->type == OperationType::NewPartTable)
            continue;  //skip create new part table

        if (operation->new_partition->mount_point == mount_point) {
            if (operation->type == OperationType::MountPoint) {
                // TODO(xushaohua): move to operation.h
                // Remove MountPointOperation with same mount point.
                it = operations_.erase(it);
                return;
            }
            else {
                // Clear mount point of old operation.
                operation->new_partition->mount_point = "";
                qDebug() << "Clear mount-point of operation:" << operation;
                return;
            }
        }
    }
}

void Delegate::updateMountPoint(const Partition::Ptr partition,
                                const QString&       mount_point)
{
    qDebug() << Q_FUNC_INFO << partition->path << mount_point;

    // Reset mount-point of operation with the same mount-point.
    resetOperationMountPoint(mount_point);

    if (!mount_point.isEmpty()) {
        // Append MountPointOperation only if |mount_point| is not empty.
        Partition::Ptr new_partition(newPartition(*partition));
        new_partition->mount_point = mount_point;
        Device::Ptr device = findDevice(partition->device_path);
        if (device.isNull()) {
            return;
        }
        // No need to update partition status.
        Operation::Ptr operation(newOperation(OperationType::MountPoint, partition, new_partition));
        operations_.append(operation);
        operation->applyToVisual(device);
    }
    else {
        partition->mount_point = "";
    }
}

bool Delegate::reCalculateExtPartBoundary(PartitionAction       action,
                                          const Partition::Ptr& current,
                                          qint64&               start_sector,
                                          qint64&               end_sector)
{
    const int device_index = DeviceIndex(virtualDevices(), current->device_path);
    if (device_index == -1) {
        return false;
    }

    Device::Ptr    device     = virtualDevices()[device_index];
    PartitionList& partitions = device->partitions;
    return reCalculateExtPartBoundary(partitions, action, current, start_sector,
                                      end_sector);
}

bool Delegate::reCalculateExtPartBoundary(const PartitionList&  partitions,
                                          PartitionAction       action,
                                          const Partition::Ptr& current,
                                          qint64&               start_sector,
                                          qint64&               end_sector)
{
    if (partitions.length() == 0) {
        return false;
    }

    bool start_found = false;
    bool end_found   = false;
    for (const Partition::Ptr& p : partitions) {
        if (p->type != PartitionType::Logical) {
            continue;
        }

        if (p == current) {
            if (action == PartitionAction::RemoveLogicalPartition) {
                continue;
            }
        }

        if (!start_found || start_sector > p->start_sector) {
            start_found                    = true;
            const qint64 oneMebiByteSector = 1 * kMebiByte / p->sector_size;
            start_sector                   = p->start_sector - oneMebiByteSector;
        }

        if (!end_found || end_sector < p->end_sector) {
            end_found  = true;
            end_sector = p->end_sector;
        }
    }

    return start_found && end_found;
}

Device::Ptr Delegate::findDevice(const QString& devicePath) {
    const int device_index = DeviceIndex(virtual_devices_, devicePath);
    if (device_index == -1) {
        qCritical() << "createPartition() device index out of range:" << devicePath;
        return nullptr;
    }

    Device::Ptr device = virtual_devices_[device_index];

    return device;
}
