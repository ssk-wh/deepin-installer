#include "continent_model.h"

using namespace installer;

ContinentModel::ContinentModel(QStringListModel *parent)
    : QStringListModel(parent)
{
}

QVariant ContinentModel::data(const QModelIndex &index, int role) const
{
    if (static_cast<IconDisplayControl>(role) == IconDisplayControl::Display) {
        return true;
    }

    return QStringListModel::data(index, role);
}


