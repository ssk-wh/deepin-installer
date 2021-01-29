#include "package_manager_model.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QDebug>
#include <QProcess>

namespace installer {

PackageRemoveModel *PackageRemoveModel::instance()
{
    static PackageRemoveModel m;
    return &m;
}

QString PackageRemoveModel::getErr()
{
    // m_packageList中剩余的包都是没有被系统卸载的
    return QObject::tr("Uninstall failed package list:") + m_packageList.join(" ");
}

bool PackageRemoveModel::start()
{
    bool reset = true;

    // 用于隔离未发布的模块内部提测
    if (GetSettingsBool(kSystemModuleDebug)) {
        for (QString pack : QStringList(m_packageList)) {
            if (!removePacakge(pack)) {
                reset = false;

            } else {
                m_packageList.removeAll(pack);  // 清理卸载成功的包
            }
        }
    }

    return reset;
}

void PackageRemoveModel::init()
{
    // 初始化配置
    m_timeout = GetSettingsInt(kSystemTimeout);
    m_packageList = GetSettingsString("DI_UNINSTALL_PACKAGES").split(" ", QString::SkipEmptyParts);
}

bool PackageRemoveModel::removePacakge(const QString &file)
{
    QString program = "apt-get";
    QStringList args = {"-y", "purge", file};

    QProcess* process = new QProcess;
    process->setProgram(program);
    process->setArguments(args);
    process->start();

    // 处理命令超时
    if (!process->waitForFinished(m_timeout)) {
        qWarning() << QString("remove package %1 timeout %2")
                       .arg(file, QString::number(m_timeout));
        return false;
    }

    // 检查执行命令时的错误
    if (process->exitStatus() != QProcess::NormalExit ||
            process->exitCode() != 0) {
        qWarning() << QString("remove package %1 failed: %2")
                       .arg(file, QString(process->readAllStandardError()));
        return false;
    }

    qDebug() << QString("remove package %1 log: ").arg(file) << process->readAllStandardOutput();

    return true;
}

PackageRemoveModel::PackageRemoveModel()
{
    init();
}

}


