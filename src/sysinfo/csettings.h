#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QVector>
#include <QMap>

namespace installer {

class CSettings
{
public:
    struct SettingInfo {
        QString other;
        QString daemon;
        QMap<QString, QString> data;
    };

public:
    CSettings(const QString &file);
    void setValue(const QString & daemon,const QString &key, const QString &value);
    QString value(const QString & daemon,const QString &key);

private:
    void read();
    void write();

    bool isDaemon(const QString& text);
    bool isData(const QString& text);

    QString key(const QString &text);
    QString value(const QString &text);
    QString toString(const QVector<SettingInfo>& data);
    void setData(const QString & daemon,const QString &key, const QString &value);

private:
    QVector<SettingInfo> m_data;
    QString              m_file;
};

}

#endif // CSETTINGS_H
