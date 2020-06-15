#ifndef STYLE_DELEGATE_H
#define STYLE_DELEGATE_H

class QScrollArea;
class QWidget;

namespace installer {

class StyleDelegate
{
public:
    static QScrollArea* area(QWidget *widget);

private:
    StyleDelegate() = delete;
    StyleDelegate(const StyleDelegate&) = delete;
};

}

#endif // STYLE_DELEGATE_H
