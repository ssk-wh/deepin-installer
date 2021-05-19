#ifndef SHADOW_WIDGET_H
#define SHADOW_WIDGET_H

#include <QWidget>
#include <QResizeEvent>

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
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    BaseFrameInterface* m_childFrameInterface;
    QVBoxLayout* m_centerLayout;
    QWidget* m_centerWidget;
};
}
#endif // PARTITION_SHADOW_WIDGET_H
