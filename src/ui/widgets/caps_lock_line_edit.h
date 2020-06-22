#ifndef CAPS_LOCK_LINE_EDIT_H
#define CAPS_LOCK_LINE_EDIT_H

#include <DLineEdit>

DWIDGET_USE_NAMESPACE

class QAction;

namespace installer {

class CapsLockLineEdit : public DLineEdit
{
    Q_OBJECT
public:
    explicit CapsLockLineEdit(QWidget *parent = nullptr);

public:
    void updateCapsLockState(bool capsLock);

protected:
    void focusOutEvent(QFocusEvent *event) override;

private:
    void initUI();
    void initConnection();

private:
    QAction* m_capsLock = nullptr;
};

}

#endif // CAPS_LOCK_LINE_EDIT_H
