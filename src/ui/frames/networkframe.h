#ifndef NETWORKFRAME_H
#define NETWORKFRAME_H

#include "ui/interfaces/frameinterface.h"

class QRegularExpressionValidator;
class QLabel;
class QPushButton;

namespace installer {
class NetworkEditWidget;

class NetworkFrame : public FrameInterface {
    Q_OBJECT
public:
    explicit NetworkFrame(FrameProxyInterface *frameProxyInterface, QWidget* parent = nullptr);

signals:
    void requestNext();

protected:
    bool event(QEvent *event) override;

private:
    QLabel*                                      m_subTitle;
    QPushButton*                                 m_nextButton;
    NetworkEditWidget*                           m_currentNetworkEditWidget;

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;

    void saveConf();
    void onDeviceSelected();
};
}  // namespace installer

#endif  // NETWORKFRAME_H
