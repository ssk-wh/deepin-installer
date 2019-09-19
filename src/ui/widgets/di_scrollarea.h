#pragma once

#include <QScrollArea>

namespace installer {

class DIScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    DIScrollArea(QWidget* parent = nullptr);

    void setWidget(QWidget *widget);

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void initUI();
    void initConnections();

    QScrollBar* m_scrollBar = nullptr;
};

}
