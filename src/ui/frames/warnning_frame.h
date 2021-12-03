#ifndef WARNNING_WIDGET_H
#define WARNNING_WIDGET_H

#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>

#include "ui/interfaces/frameinterface.h"

namespace installer {

class CommentLabel;

class WarnningFrame : public ChildFrameInterface
{
    Q_OBJECT

public:
    explicit WarnningFrame(FrameProxyInterface* frameProxyInterface,
                            QWidget* parent = nullptr);

public:
    void setShowIcon(const QString &showicon);
    void setTitle(const QString &titletext);
    void setComment(const QString &commenttext);
    void setEnterButtonText(const QString &buttontext);
    void setCancelButtonText(const QString &buttontext);
    void useTitle(bool isuse);
    void useComment(bool isuse);
    void useEnterButton(bool isuse);
    void useCancelButton(bool isuse);
    void setEnterButtonStyle(const QString &buttonstyle);
    void setCancelButtonStyle(const QString &buttonstyle);

    bool focusSwitch() override;
    bool doSpace() override;
    bool doSelect() override;
    bool directionKey(int keyvalue) override;

signals:
    void quitclosed();
    void quitEntered();
    void quitCanceled();


protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void initConnections();
    void initUI();

private:
    QPushButton *m_icoButton = nullptr;
    QPushButton *m_closeButton = nullptr;
    QString m_titleText = "";
    QString m_commentText = "";
    QString m_enterButtonText = "";
    QString m_cancelButtonText = "";
    QLabel *m_titleLabel = nullptr;
    CommentLabel *m_commentLabel = nullptr;
    QPushButton *m_enterButton = nullptr;
    QLabel *m_spliLabel = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QHBoxLayout *m_buttonGroupLayout = nullptr;
};

}

#endif // WARNNING_WIDGET_H
