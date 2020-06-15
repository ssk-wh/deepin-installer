#ifndef SELECT_BUTTON_H
#define SELECT_BUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE

namespace installer {

class SelectButton : public DPushButton
{
    Q_OBJECT

public:
    SelectButton(QWidget* parent = nullptr);

public:
    void setSelect(bool select);
};

}

#endif // SELECT_BUTTON_H
