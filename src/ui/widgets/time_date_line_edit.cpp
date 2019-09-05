#include "time_date_line_edit.h"

namespace installer {

TimeDateLineEdit::TimeDateLineEdit(QWidget* parent)
    : QLineEdit (parent)
{
}

void TimeDateLineEdit::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    QLineEdit::focusOutEvent(event);
    emit lostFocus();
}

}
