#ifndef FRAMEINTERFACEPRIVATE_H
#define FRAMEINTERFACEPRIVATE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QShortcut>
#include <QTimer>

#include "frameinterface.h"
#include "service/language_manager.h"

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
        mainLayout->addWidget(nextButton, 0, Qt::AlignCenter | Qt::AlignBottom);

        frameInterface->setLayout(mainLayout);

        connect(parent, &FrameInterface::updateNextButton, this,
                &FrameInterfacePrivate::updateNextButton);
        connect(nextButton, &QPushButton::clicked, this, &FrameInterfacePrivate::onNextButtonClickHandle);
        QTimer::singleShot(0, this, &FrameInterfacePrivate::registerShortcutKey);

        // Register Next button text
        LanguageManager::translator(nextButton, &QPushButton::setText, TranslatorType::NextButton);
    }

    // Verify that jumping to the next frame is allowed.
    virtual bool validate() const {
        return true;
    }

    virtual void updateNextButton() const {
        nextButton->setEnabled(true);
    }

    virtual void onNextButtonClickHandle() const {
        FrameInterface* parent = dynamic_cast<FrameInterface*>(this->parent());
        if (parent != nullptr) {
            if (validate()) {
                parent->nextFrame();
            }
        }
    }

    virtual void registerShortcutKey() const {
        QShortcut *key = new QShortcut(QKeySequence(Qt::Key_Return), nextButton);
        key->setAutoRepeat(false);
        connect(key, &QShortcut::activated, this, [=]{
            emit nextButton->click();
        });
    }

protected:
    QVBoxLayout* centerLayout;
    FrameInterface* frameInterface;

    QPushButton* nextButton;
};

}

#endif // FRAMEINTERFACEPRIVATE_H
