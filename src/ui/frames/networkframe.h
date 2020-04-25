#ifndef NETWORKFRAME_H
#define NETWORKFRAME_H

#include "ui/interfaces/frameinterface.h"
#include "ui/delegates/network_operate.h"

#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class QRegularExpressionValidator;
class QLabel;
class QPushButton;
class QVBoxLayout;

namespace installer {
class NetworkEditWidget;

class NetworkFrame : public FrameInterface {
    Q_OBJECT
public:
    explicit NetworkFrame(FrameProxyInterface *frameProxyInterface, QWidget* parent = nullptr);

    QString returnFrameName() const override;

    void initDeviceWidgetList();

signals:
    void requestNext();

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent* event) override;

private:
    QVBoxLayout*                                m_leftLayout;
    QLabel*                                     m_subTitle;
    QPushButton*                                m_nextButton;
    NetworkEditWidget*                          m_currentNetworkEditWidget;
    DButtonBox*                                 m_buttonBox = nullptr;
    QList<DButtonBoxButton*>                    m_buttonList;
    QMap<NetworkManager::Device::Ptr, NetworkSettingInfo> m_deviceConfigInfo;

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;

    void saveConf();
    void onButtonGroupToggled(QAbstractButton *button);
};
}  // namespace installer

#endif  // NETWORKFRAME_H
