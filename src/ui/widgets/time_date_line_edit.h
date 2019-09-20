#pragma once

#include <QLineEdit>

namespace installer {

class TimeDateLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    TimeDateLineEdit(QWidget* parent = nullptr);

signals:
    void lostFocus();

protected:
    void focusOutEvent(QFocusEvent* event) override;
};

}

