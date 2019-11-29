#ifndef CONTROLPLATFORMFRAME_H
#define CONTROLPLATFORMFRAME_H

#include "ui/models/control_platform_region_model.h"

#include "ui/interfaces/frameinterface.h"

#include <QUrl>
#include <QWidget>

class QComboBox;
class QNetworkReply;
class QVBoxLayout;
namespace installer {
class LineEdit;
class NavButton;
class ControlPlatformRegionModel;
class TableComboBox;
class TitleLabel;
class CommentLabel;
class ControlPlatformFrame : public FrameInterface {
    Q_OBJECT
public:
    ControlPlatformFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);

    virtual void init() override;
    virtual void finished() override;
    virtual bool shouldDisplay() const override;

protected:
    bool event(QEvent* event) override;

private:
    void onNetworkFinished(QNetworkReply* reply);
    void onNextClicked();
    void onRegionSelected();

private slots:
    void onNetworkStateChanged();

private:
    TitleLabel*                 m_titleLbl;
    CommentLabel*               m_subTitleLbl;
    LineEdit*                   m_serverLineEdit;
    TableComboBox*              m_regionBox;
    NavButton*                  m_nextButton;
    ControlPlatformRegionModel* m_regionModel;
    QUrl                        m_serverUrl;
    QList<RegionInfo>           m_regionInfo;
    QVBoxLayout*                m_macInfoLayout;
    QVBoxLayout*                m_ipInfoLayout;
};
}  // namespace installer
#endif  // !CONTROLPLATFORMFRAME_H
