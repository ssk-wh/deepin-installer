#ifndef PACKAGE_MANAGER_MODEL_H
#define PACKAGE_MANAGER_MODEL_H

#include <QString>
#include <QObject>

namespace installer {

class PackageRemoveModel
{
public:
    static PackageRemoveModel* instance();

public:
    bool start();
    QString getErr();

private:
    void init();
    void setErr(const QString &err);
    bool removePacakge(const QString &file);

private:
    PackageRemoveModel();
    PackageRemoveModel(const PackageRemoveModel&) = delete;
    PackageRemoveModel& operator=(const PackageRemoveModel&) = delete;

private:
    QStringList m_packageList;
    QString m_error;
    int m_timeout = -1;    // 默认超时时间是
};

}

#endif // PACKAGE_MANAGER_MODEL_H
