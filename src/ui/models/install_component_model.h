#pragma once

#include <QAbstractListModel>

namespace installer {

class InstallComponentListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit InstallComponentListModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;

private:
    QList<QString> m_dataList;
};

}
