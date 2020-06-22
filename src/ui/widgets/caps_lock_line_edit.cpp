#include "caps_lock_line_edit.h"

#include "ui/utils/keyboardmonitor.h"

#include <QAction>

installer::CapsLockLineEdit::CapsLockLineEdit(QWidget *parent):
    DLineEdit(parent)
{
    initUI();
    initConnection();
}

void installer::CapsLockLineEdit::updateCapsLockState(bool capsLock)
{
    if (capsLock) {
        lineEdit()->addAction(m_capsLock, QLineEdit::TrailingPosition);
    } else {
        lineEdit()->removeAction(m_capsLock);
    }
    lineEdit()->update();
}

void installer::CapsLockLineEdit::focusOutEvent(QFocusEvent *event)
{
    if (m_capsLock != nullptr) {
        lineEdit()->removeAction(m_capsLock);
        lineEdit()->update();
    }

    return DLineEdit::focusOutEvent(event);
}

void installer::CapsLockLineEdit::initUI()
{
    m_capsLock = new QAction(this);
    m_capsLock->setIcon(QIcon(":/images/capslock.svg"));
}

void installer::CapsLockLineEdit::initConnection()
{
    connect(KeyboardMonitor::instance(),
            &KeyboardMonitor::capslockStatusChanged, this,
            &CapsLockLineEdit::updateCapsLockState);
}
