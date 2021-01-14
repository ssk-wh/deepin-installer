#pragma once

#include <QCursor>

class MouseShape
{
public:
    MouseShape();

    void setCursor(const Qt::CursorShape shape);
    void resetCursor();

private:
    bool m_setOverrideCursor = false;
};
