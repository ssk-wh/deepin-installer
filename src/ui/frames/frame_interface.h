#pragma once

#include <QWidget>

namespace installer {
class FrameInterface : public QWidget {
    Q_OBJECT
public:
    explicit FrameInterface(QWidget* parent = nullptr) : QWidget(parent) {}

    virtual void readConf() {}
    virtual void writeConf() {}
};
}  // namespace installer
