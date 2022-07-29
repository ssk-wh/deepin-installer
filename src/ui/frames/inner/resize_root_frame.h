#pragma once

#include <QDialog>
#include <memory>

#include "partman/partition.h"
#include "ui/interfaces/frameinterface.h"

namespace installer {
class FrameProxyInterface;
class FullDiskDelegate;
}  // namespace installer
class ResizeRootFramePrivate;
class ResizeRootFrame : public installer::ChildFrameInterface {
    Q_OBJECT
public:
    ResizeRootFrame(installer::FrameProxyInterface* inter,
                    installer::FullDiskDelegate*    delegate,
                    QWidget*                        parent = nullptr);
    ~ResizeRootFrame() override;

signals:
    void finished();
    void canceled();

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    std::unique_ptr<ResizeRootFramePrivate> d_ptr;
};
