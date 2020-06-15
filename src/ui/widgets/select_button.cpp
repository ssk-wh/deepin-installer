#include "select_button.h"

#include "base/file_util.h"

installer::SelectButton::SelectButton(QWidget *parent):
    DPushButton(parent)
{

}

void installer::SelectButton::setSelect(bool select)
{
    if (select) {
        this->setStyleSheet(ReadFile(":/styles/select_button.css"));
    } else {
        this->setStyleSheet("");
    }
}
