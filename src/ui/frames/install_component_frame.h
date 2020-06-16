#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMap>
#include <QLabel>
#include <QScopedPointer>

#include "ui/interfaces/frameinterface.h"

namespace installer {

class NavButton;
class ComponentStruct;
struct ComponentInfo;
class ComponentWidget;
class DIScrollArea;
class TitleLabel;
class SelectInstallComponentFramePrivate;

class SelectInstallComponentFrame : public FrameInterface
{
    Q_OBJECT

    friend SelectInstallComponentFramePrivate;

public:
    SelectInstallComponentFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~SelectInstallComponentFrame() override;

    // Read default install components.
    void init() override;

    // Write install components to settings file.
    void finished() override;

    // Read the configuration file to confirm that the current page is available
    bool shouldDisplay() const override;

    QString returnFrameName() const override;

protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    QScopedPointer<SelectInstallComponentFramePrivate> m_private;
};

}
