#pragma once

#include <QFrame>

class QRadioButton;
class QLabel;
class QHBoxLayout;
class QDebug;

namespace installer {
enum class FrameLabelState{
    Initial,
    Show,
    FinishedConfig
};

class FrameInterface;

class FrameSelectedWidget : public QFrame
{
    Q_OBJECT
public:
    explicit FrameSelectedWidget(FrameInterface* frame, QWidget *parent = nullptr);

    void setText(const QString& text);
    void setNormalStyle();
    void setShowStyle();
    void setBackable(bool backable);
    bool isBackable() const;

signals:
    void frameClicked(FrameInterface* frame);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    FrameLabelState m_state = FrameLabelState::Initial;
    FrameInterface* m_frame = nullptr;

    QLabel* m_noLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_checkedLabel = nullptr;
    QHBoxLayout* m_hLayout = nullptr;
};
}

