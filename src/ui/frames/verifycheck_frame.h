//
// Created by dtx on 2022/9/26.
//

#ifndef VERIFYCHECKFRAME_H
#define VERIFYCHECKFRAME_H

#include "ui/interfaces/frameinterface.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include <QWidget>
#include <QLabel>

namespace installer {

class Verify : public QObject
{
    Q_OBJECT
public:
    void start();

signals:
    void done(bool flag);
};

class VerifyCheckFramePrivate;

class VerifyCheckFrame : public FrameInterface {
    Q_OBJECT
    friend VerifyCheckFramePrivate;

public:
    VerifyCheckFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~VerifyCheckFrame() override;

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;
    QString returnFrameName() const override;

private:
    void verifyCheck();

protected:
    void changeEvent(QEvent* event) override;
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool focusSwitch() override;
    bool doSpace() override;
    bool doSelect() override;
    bool directionKey(int keyvalue) override;

signals:
    void verifyStart();
    void verifyDone(bool isOk);

private:
    Verify verify;
    QScopedPointer<VerifyCheckFramePrivate> m_private;
};

}


#endif //VERIFYCHECKFRAME_H
