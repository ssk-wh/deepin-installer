#ifndef FRAMEINTERFACEPRIVATE_H
#define FRAMEINTERFACEPRIVATE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "frameinterface.h"

namespace installer {

class FrameInterfacePrivate :public QObject
{
    Q_OBJECT

    friend FrameInterface;

public:
    explicit FrameInterfacePrivate(FrameInterface* parent)
        : QObject(parent)
        , centerLayout(new QVBoxLayout)
        , frameInterface(parent)
        , nextButton(new QPushButton)
    {
        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->addLayout(centerLayout);

        mainLayout->addWidget(nextButton);

        frameInterface->setLayout(mainLayout);

        connect(nextButton, &QPushButton::clicked, parent, &FrameInterface::nextFrame);
    }

protected:
    QVBoxLayout* centerLayout;
    FrameInterface* frameInterface;

    QPushButton* nextButton;
};

}

#endif // FRAMEINTERFACEPRIVATE_H
