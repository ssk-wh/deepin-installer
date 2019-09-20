#ifndef CONTINENT_MODEL_H
#define CONTINENT_MODEL_H

#include <QStringListModel>

namespace installer {

class ContinentModel : public QStringListModel
{
    Q_OBJECT
public:
    enum IconDisplayControl {
        NotDisplay = Qt::UserRole + 1,
        Display,
        Invalid,
    };

    ContinentModel(QStringListModel* parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

}
#endif // CONTINENT_MODEL_H

Q_DECLARE_METATYPE(installer::ContinentModel::IconDisplayControl);
