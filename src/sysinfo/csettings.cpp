#include "csettings.h"

#include <QFile>
#include <QDebug>
#include <QRegExp>

installer::CSettings::CSettings(const QString &file)
    :m_file(file)
{
    read();
}

void installer::CSettings::setValue(const QString &daemon, const QString &key, const QString &value)
{
    QRegExp exp(QString("^[ ]{0,}[#]{0,}[ ]{0,}%1$").arg(key)); // 匹配key是否被注释或者已经存在文件中
    for (SettingInfo& d : m_data) {
        if (d.daemon == QString("[%1]").arg(daemon)) {
            QMap<QString, QString> tmp;
            for (QString k : d.data.keys()) {
                if (k.contains(exp)) {
                    tmp[key] = value;  // 处理key被注释的情况
                } else {
                    tmp[k] = d.data.value(k);
                }
            }
            if (!tmp.keys().contains(key)) {
                tmp[key] = value;     //  处理ini文件中不存在的key
            }
            d.data = tmp;
            break;                    //  减少循环次数
        }
    }

    write();
}

QString installer::CSettings::value(const QString &daemon, const QString &key)
{
    QString reset;
    for (SettingInfo& d : m_data) {
        if (d.daemon == QString("[%1]").arg(daemon)) {
            if (d.data.keys().contains(key)) {
                reset = d.data.value(key);
                break;
            }
        }
    }

    return reset;
}

QString installer::CSettings::key(const QString &text)
{
    QStringList list = text.split("=", QString::SkipEmptyParts);
    return list.isEmpty() ? QString() : list.at(0);
}

QString installer::CSettings::value(const QString &text)
{
    QStringList list = text.split("=", QString::SkipEmptyParts);
    return list.size() != 2 ? QString() : list.at(1);
}

QString installer::CSettings::toString(const QVector<SettingInfo> &data)
{
    // 格式化数据，为数据持久化做准备
    QString reset;
    for (SettingInfo d : data) {
        if (!d.other.isEmpty()) {
            reset += d.other + "\n";
        }
        if (!d.daemon.isEmpty()){
            reset += d.daemon + "\n";
        }

        for (QString k : d.data.keys()) {
            reset += k + "=" + d.data.value(k) + "\n";
        }
    }

    return reset;
}

void installer::CSettings::read()
{
    QFile f(m_file);
    if (!f.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open file. " << m_file;
        return;
    }

    SettingInfo info;
    while (!f.atEnd()) {

        QString line = f.readLine().trimmed();

        // 对文件内容进行分类

        /* 处理域类型的数据 */
        if (isDaemon(line)) {
            /* 保存上一个域的key-value数据 */
            if (!info.data.isEmpty()) {
                m_data.append(info);
                info = SettingInfo();
            }

            /* 重新开始记录域数据 */
            info.daemon = line;

        /* 处理key-value数据 */
        } else if (!info.daemon.isEmpty() && isData(line)) {
            QString k = key(line);
            QString val = value(line);
            info.data.insert(k, val);

        } else {
            /* 保存上一个域的key-value数据 */
            if (!info.daemon.isEmpty()) {
                if (!info.data.isEmpty()) {
                    m_data.append(info);
                    info = SettingInfo();
                }
            }

            /* 处理不在域中的数据 */
            info.other = line;
            m_data.append(info);
            info = SettingInfo();
        }
    }

    /* 保存最后一次循环的key-value */
    if (!info.data.isEmpty()) {
        m_data.append(info);
    }
}

void installer::CSettings::write()
{
    // 数据持久化
    QFile f(m_file);
    if (!f.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to open file. " << m_file;
        return;
    }

    QByteArray text = toString(m_data).toUtf8();
    f.write(text);
}

bool installer::CSettings::isDaemon(const QString &text)
{
    return text.contains("[") && text.contains("]") && !text.contains("#");
}

bool installer::CSettings::isData(const QString &text)
{
    QRegExp exp("=");
    return text.contains(exp);
}
