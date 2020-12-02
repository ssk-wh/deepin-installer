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
class CommentLabel;
class TitleLabel;

class NetworkFrame : public FrameInterface {
    Q_OBJECT
public:
    explicit NetworkFrame(FrameProxyInterface *frameProxyInterface, QWidget* parent = nullptr);

    static NetworkOperate* getNetworkOperateByDeviceUdi(const QString& udi);

    QString returnFrameName() const override;

    void initDeviceWidgetList();
    void shockDdeDaemon();

signals:
    void requestNext();

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent* event) override;

private:
    static QMap<QString, NetworkOperate*>       m_mapDeviceUdi2NetworkOperate;

    TitleLabel* title_label_ = nullptr;
    CommentLabel* comment_label_ = nullptr;
    QVBoxLayout*                                m_leftLayout;
    QPushButton*                                m_nextButton;
    NetworkEditWidget*                          m_currentNetworkEditWidget;
    DButtonBox*                                 m_buttonBox = nullptr;
    QList<DButtonBoxButton*>                    m_buttonList;
    QMap<NetworkManager::Device::Ptr, NetworkSettingInfo> m_deviceConfigInfo;
    QStringList                                 m_connectionUuidList;
    QMap<QString, bool>                         m_mapDeviceUdi2InUsed;

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;

    void saveConf();
    void updateTs();
    void onButtonGroupToggled(QAbstractButton *button);
    QStringList getAllConnectionUuids();
    void deleteOtherConnections();
};
}  // namespace installer

#endif  // NETWORKFRAME_H
