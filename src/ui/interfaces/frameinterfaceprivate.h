#ifndef FRAMEINTERFACEPRIVATE_H
#define FRAMEINTERFACEPRIVATE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "frameinterface.h"

#define NEXTBTN_WIDTH 310
#define NEXTBTN_HEIGHT 36

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
        centerLayout->setContentsMargins(0, 0, 0, 0);
        centerLayout->setSpacing(0);

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->addLayout(centerLayout);

        nextButton->setFixedSize(NEXTBTN_WIDTH, NEXTBTN_HEIGHT);
        mainLayout->addWidget(nextButton, 0, Qt::AlignCenter);

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
