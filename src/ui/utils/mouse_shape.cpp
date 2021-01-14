#include "mouse_shape.h"

#include <QApplication>

MouseShape::MouseShape()
    : m_setOverrideCursor(false)
{
}

void MouseShape::setCursor(const Qt::CursorShape shape)
{
    if (!m_setOverrideCursor) {
        QApplication::setOverrideCursor(QCursor(shape));
        m_setOverrideCursor = true;
    }
}

void MouseShape::resetCursor()
{
    if (m_setOverrideCursor) {
        QApplication::restoreOverrideCursor();
        m_setOverrideCursor = false;
    }
}
