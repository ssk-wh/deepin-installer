#ifndef SHADOW_WIDGET_H
#define SHADOW_WIDGET_H

#include <QWidget>
#include <QResizeEvent>

class QStackedLayout;
class QVBoxLayout;

namespace installer {
class BaseFrameInterface;
class ShadowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowWidget(QWidget* parent = nullptr);

    void setContent(BaseFrameInterface* inter);
    void eraseContent();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    BaseFrameInterface* m_childFrameInterface;
    QStackedLayout* m_centerLayout;

    QWidget* m_widget;
};
}
#endif // PARTITION_SHADOW_WIDGET_H
