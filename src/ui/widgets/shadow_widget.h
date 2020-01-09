#ifndef SHADOW_WIDGET_H
#define SHADOW_WIDGET_H

#include <QWidget>
#include <QResizeEvent>

class QVBoxLayout;

namespace installer {
class ChildFrameInterface;
class ShadowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowWidget(QWidget* parent = nullptr);

    void setContent(ChildFrameInterface* inter);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    ChildFrameInterface* childFrameInterface;
    QVBoxLayout* m_centerLayout;
};
}
#endif // PARTITION_SHADOW_WIDGET_H