#ifndef CONTROLPLATFORMFRAME_H
#define CONTROLPLATFORMFRAME_H

#include "ui/models/control_platform_region_model.h"

#include "ui/interfaces/frameinterface.h"

#include <QUrl>
#include <QWidget>
#include <QScopedPointer>

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
class ControlPlatformFramePrivate;
class ControlPlatformFrame : public FrameInterface {
    Q_OBJECT
    friend ControlPlatformFramePrivate;
public:
    ControlPlatformFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~ControlPlatformFrame() override;
    virtual void init() override;
    virtual void finished() override;
    virtual bool shouldDisplay() const override;
    QString returnFrameName() const override;


protected:
    bool event(QEvent* event) override;

private:
    QScopedPointer<ControlPlatformFramePrivate> m_private;
};

}  // namespace installer
#endif  // !CONTROLPLATFORMFRAME_H
