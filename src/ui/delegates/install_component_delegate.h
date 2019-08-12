#pragma once

#include <QStyledItemDelegate>

namespace installer {

class InstallComponentDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit InstallComponentDelegate(QObject *parent = nullptr);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const;
};

}
