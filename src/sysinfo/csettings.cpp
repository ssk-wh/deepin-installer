#include "csettings.h"

#include <QFile>
#include <QDebug>
#include <QRegExp>
#include <QSettings>

installer::CSettings::CSettings(const QString &file)
    :m_file(file)
{
    read();
}

void installer::CSettings::setValue(const QString &daemon, const QString &key, const QString &value)
{
    setData(QString("[%1]").arg(daemon), key, value);
}

void installer::CSettings::setData(const QString &daemon, const QString &key, const QString &value)
{
    bool is_find = false;
    QRegExp exp(QString("^[#]{1,}[ ]{0,}%1$").arg(key)); // 匹配key是否被注释
    for (SettingInfo& d : m_data) {
        if (d.daemon == daemon) {
            QMap<QString, QString> tmp;
            for (QString k : d.data.keys()) {
                if (k == key) {
                    tmp[k] = value;                 // 更新key
                }
                else if (k.contains(exp)) {
                    tmp[key] = value;               // 处理key被注释的情况

                } else {
                    tmp[k] = d.data.value(k);       // 填充剩余的key-value
                }
            }

            if (tmp.keys().indexOf(key) == -1) {
                tmp[key] = value;                   // 处理key不在daemon下的情况
            }

            d.data = tmp;
            is_find = true;

            break;                    //  减少循环次数
        }
    }

    // 处理daemon不在文件中的情况
    if (!is_find) {
        SettingInfo d;
        d.daemon = daemon;
        d.data[key] = value;
        m_data.push_back(d);
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
    int pos = text.indexOf("=");
    // 剔除掉key中前后空格
    return text.left(pos).trimmed();
}

QString installer::CSettings::value(const QString &text)
{
    int pos = text.indexOf("=") + 1;
    // 剔除掉value中空格和引号
    return text.right(text.size() - pos).trimmed()\
            .remove(QRegExp("\""))\
            .remove(QRegExp("\n")).trimmed();
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
    if (!f.open(QIODevice::ReadWrite)) {
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

    f.close();  // 设置打开文件标志为ReadWrite，在close文件时，如果文件不存在则文件会被创建

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
    f.close();
}

bool installer::CSettings::isDaemon(const QString &text)
{
    return text.contains("[") && text.contains("]") && !text.contains("#") && !text.contains("=");
}

bool installer::CSettings::isData(const QString &text)
{
    QRegExp exp("=");
    return text.contains(exp);
}
