#include "install_component_delegate.h"

namespace installer {

InstallComponentDelegate::InstallComponentDelegate(QObject *parent)
    : QStyledItemDelegate (parent)
{
}

void InstallComponentDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
}

}
