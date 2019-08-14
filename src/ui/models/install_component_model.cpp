#include "install_component_model.h"

namespace installer {

InstallComponentListModel::InstallComponentListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    this->setObjectName("install_component_model");
    m_dataList << "server1" << "server2" << "server3";
}

QVariant InstallComponentListModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();
    if(!index.isValid())
        return  QVariant();
    if(index.row() >= m_dataList.count())
        return QVariant();

    return m_dataList.at(index.row());
}

int InstallComponentListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_dataList.length();
}

void InstallComponentListModel::initData()
{
    beginResetModel();
    m_dataList << "ss1" << "ss2" << "ss3";
    this->endResetModel();
}

}
