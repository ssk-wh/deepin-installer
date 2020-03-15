#ifndef SHADOW_WIDGET_H
#define SHADOW_WIDGET_H

#include <QWidget>
#include <QResizeEvent>

class QStackedLayout;
class QVBoxLayout;

namespace installer {
class ChildFrameInterface;
class ShadowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowWidget(QWidget* parent = nullptr);

    void setContent(ChildFrameInterface* inter);
    void eraseContent();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    ChildFrameInterface* childFrameInterface;
    QStackedLayout* m_centerLayout;
    QVBoxLayout* m_mainLayout;
};
}
#endif // PARTITION_SHADOW_WIDGET_H
