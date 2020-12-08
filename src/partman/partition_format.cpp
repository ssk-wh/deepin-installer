/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "partman/partition_format.h"

#include <QDebug>
#include <memory>

#include "base/command.h"
#include "sysinfo/machine.h"
#include "ui/delegates/partition_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

namespace installer {
class PartitionFormater {
public:
    explicit PartitionFormater(Partition::Ptr partition)
        : m_partition(partition) {
        };

    virtual ~PartitionFormater() {};

    virtual QString command() {
        return FsFormatCmdMap[m_partition->fs];
    }

    virtual QStringList args() {
      return {};
    }

    inline bool exec() {
        return SpawnCmd(command(), args());
    };

    inline bool isLabelEmpty() {
      return label().isEmpty();
    }

    inline QString label() {
      return m_partition->label;
    }

    inline QString path() {
      return m_partition->path;
    }

private:
    Partition::Ptr m_partition;
};

class BtrfsFormater : public PartitionFormater {
public:
    using PartitionFormater::PartitionFormater;

    virtual QStringList args() override
    {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(255);
            args << QStringList{ "-L", real_label };
        }

        return args;
    }
};

class Ext2Formater : public PartitionFormater {
  public:
      using PartitionFormater::PartitionFormater;

      virtual QStringList args() override
      {
          QStringList args{ "-F", path() };

          if (!isLabelEmpty()) {
              const QString real_label = label().left(16);
              args << QStringList{ "-L", real_label };
          }

          return args;
      }
};

class Ext3Formater : public Ext2Formater {
  public:
      using Ext2Formater::Ext2Formater;
};

class Ext4Formater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
          QStringList args{ "-F", path() };

          if (!isLabelEmpty()) {
              args << QStringList{ "-L", label() };
          }

          const MachineArch arch = GetMachineArch();
          if (arch == MachineArch::LOONGSON || arch == MachineArch::SW) {
              args << QStringList{ "-O ^64bit" };
          }

          return args;
  }
};

class LVMPVFormater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
          QStringList args({path(),"-y"}) ;
          return args;
  }
};

class F2FSFormater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(19);
            args << QStringList{ "-l", real_label };
        }

        return args;
  }
};

class FAT16Formater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-F16", "-v", "-I", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(11);
            args << QStringList{ "-n", real_label };
        }

        return args;
  }
};

class FAT32Formater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-F32", "-v", "-I", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(11);
            args << QStringList{ "-n", real_label };
        }

        return args;
  }
};

class NTFSFormater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-Q", "-v", "-F", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(128);
            args << QStringList{ "-L", real_label };
        }

        return args;
  }
};

class HFSFormater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(27);
            args << QStringList{ "-l", real_label };
        }

        return args;
  }
};

class HFSPLUSFormater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(63);
            args << QStringList{ "-v", real_label };
        }

        return args;
  }
};

class JfsFormater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-q", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(11);
            args << QStringList{ "-L", real_label };
        }

        return args;
  }
};

class LinuxSwapFormater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(15);
            args << QStringList{ "-L", real_label };
        }

        return args;
  }
};

class Nilfs2Formater : public PartitionFormater {
  public:
  using PartitionFormater::PartitionFormater;
  virtual QStringList args() override {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(1);
            args << QStringList{ "-L", real_label };
        }

        return args;
  }
};

class Reiser4Formater : public PartitionFormater {
public:
    using PartitionFormater::PartitionFormater;
    virtual QStringList args() override
    {
        QStringList args{ "--force", "--yes", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(16);
            args << QStringList{ "--label", real_label };
        }

        return args;
    }
};

class ReiserFsFormater : public PartitionFormater {
public:
    using PartitionFormater::PartitionFormater;
    virtual QStringList args() override
    {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(16);
            args << QStringList{ "--label", real_label };
        }

        return args;
    }
};

class XfsFormater : public PartitionFormater {
public:
    using PartitionFormater::PartitionFormater;
    virtual QStringList args() override
    {
        QStringList args{ "-f", path() };

        if (!isLabelEmpty()) {
            const QString real_label = label().left(12);
            args << QStringList{ "-L", real_label };
        }

        return args;
    }
};

// Make filesystem on |partition| based on its fs type.
bool Mkfs(const Partition::Ptr partition, bool)
{
    qDebug() << "Mkfs()" << partition;

    using Formater = std::shared_ptr<PartitionFormater>;
    QMap<FsType, std::shared_ptr<PartitionFormater>> map{
        { FsType::Btrfs, Formater(new BtrfsFormater(partition)) },
        { FsType::Ext2, Formater(new Ext2Formater(partition)) },
        { FsType::Ext3, Formater(new Ext3Formater(partition)) },
        { FsType::Ext4, Formater(new Ext4Formater(partition)) },
        { FsType::F2fs, Formater(new F2FSFormater(partition)) },
        { FsType::Fat16, Formater(new FAT16Formater(partition)) },
        { FsType::EFI, Formater(new FAT32Formater(partition)) },
        { FsType::Fat32, Formater(new FAT32Formater(partition)) },
        { FsType::Hfs, Formater(new HFSFormater(partition)) },
        { FsType::HfsPlus, Formater(new HFSPLUSFormater(partition)) },
        { FsType::Jfs, Formater(new JfsFormater(partition)) },
        { FsType::LinuxSwap, Formater(new LinuxSwapFormater(partition)) },
        { FsType::Nilfs2, Formater(new Nilfs2Formater(partition)) },
        { FsType::NTFS, Formater(new NTFSFormater(partition)) },
        { FsType::Reiser4, Formater(new Reiser4Formater(partition)) },
        { FsType::Reiserfs, Formater(new ReiserFsFormater(partition)) },
        { FsType::Xfs, Formater(new XfsFormater(partition)) },
        { FsType::Recovery, Formater(new Ext4Formater(partition)) },
        { FsType::LVM2PV, Formater(new LVMPVFormater(partition))}

    };

    if (!map.contains(partition->fs)) {
        qWarning() << "Unsupported filesystem to format!" << partition->path;
        return false;
    }

    bool reset = map[partition->fs]->exec();
    qDebug() << "Mkfs: sync start...";
    SpawnCmd("sync", {});
    qDebug() << "Mkfs: sync end.";

    return reset;
}

// Make filesystem on |partition| based on its fs type.
bool Mkfs(const Partition::Ptr partition)
{
    bool reset = Mkfs(partition, true);
    if (GetSettingsBool(kPartitionIsFsckFileSystem)) {
        Fsck(partition);
    }
    return reset;
}

void Fsck(const Partition::Ptr partition) {
    auto fs_type = [=](FsType type) {
    switch (type) {
      case FsType::Btrfs: return "btrfs";
      case FsType::Ext2: return "ext2";
      case FsType::Ext3: return "ext3";
      case FsType::Ext4: return "ext4";
      case FsType::Fat16:
      case FsType::Fat32: return "fat";
      case FsType::Jfs: return "jfs";
      case FsType::Reiserfs: return "reiserfs";
      case FsType::Xfs: return "xfs";
      default: return "";
    }};

    QString fs = fs_type(partition->fs);
    /* 目前只针对fs_type中定义的几种文件系统进行修复 */
    if (!fs.isEmpty()) {
        QString tmp = QString("/tmp/deepin-installer-fsck")
                + QDir::separator()
                + QString("fsck-fs-%1").arg(QString(partition->path).replace("/", "-"));
        QDir().rmpath(tmp);
        QDir().mkpath(tmp);

        /* 挂载分区，准备开始检查格式化的分区是否正常 */
        qDebug() << "Fsck:mount: " <<  (QStringList() << partition->path << tmp).join(" ");
        SpawnCmd("mount", QStringList() << partition->path << tmp);
        QString test = tmp + QDir::separator() + "deepin-installer-fsck-test";

        int conunt = 1;
        int MAX_COUNT = 10;  // 最多重试10次
        /* 通过在挂载的分区的文件系统中创建一个目录来检查文件系统是否格式化正常 */
        while (!QDir().mkpath(test)) {

            qDebug() << "Fsck:umount: " << partition->path;
            qDebug() << "Fsck: " << (QStringList() << "-y" << "-t" << fs
                     << partition->path).join(" ");

            /* 格式化的文件系统不正常：
             * 1. 卸载分区
             * 2. 重新格式化分区--增加修复的成功率
             * 3. 执行修复分区文件系统
            */
            SpawnCmd("umount", QStringList() << partition->path);
            Mkfs(partition, true);
            SpawnCmd("fsck", QStringList() << "-y" << "-t" << fs
                                           << partition->path);

            /* 重新挂载分区，循环检查 */
            SpawnCmd("mount", QStringList() << partition->path << tmp);
            if (++conunt > MAX_COUNT) {
                break;
            }
        }

        /* 清理 */
        QDir().rmpath(test);
        SpawnCmd("umount", QStringList() << partition->path);
    }
}

}  // namespace installer
