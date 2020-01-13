#pragma once

#include <QWidget>

class PartitionColorLabel : public QWidget
{
    Q_OBJECT
public:
    explicit PartitionColorLabel(QColor color, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color;
};
