#ifndef CLI_INSTALL_FRAMEINTERFACE_H
#define CLI_INSTALL_FRAMEINTERFACE_H
#include <QObject>
#include <QScopedPointer>
#include <QVector>
#include "frameinterfaceprivate.h"
#include <QDebug>
#include <cursesapp.h>
namespace  installer{

enum FRAME_STATE {
    FRAME_STATE_NOT_START = 0,
    FRAME_STATE_RUNNING,
    FRAME_STATE_FINISH,
    FRAME_STATE_ABORT,
    FRAME_STATE_ERROR
};



class FrameInterface : public QObject
{
    Q_OBJECT
public:
    explicit FrameInterface(FrameInterface* parent = nullptr);
    virtual ~FrameInterface();


    void addChildFrame(FrameInterface* childFrame);
    void start();
    void setCanGetChild(bool iscan);
    FrameInterface* getCurrentChild();
    virtual bool init();
    virtual void initConnection();
    virtual void show();
    virtual void hide();
    virtual void showChild(int index);
    virtual void hideChild(int index);
    virtual void hideAllChild();
    virtual bool shouldDisplay() const;
    virtual QString getFrameName() {
        return "";
    }
    FrameInterfacePrivate* getPrivate() {
        return m_private;
    }

    void setFrameState(FRAME_STATE frameState) {
        m_currState = frameState;
    }

    FRAME_STATE getFrameState() {
        return  m_currState;
    }

    virtual QString getAbout() {
        return   QString("<↑ ↓ ← →>%1  | <Tab>%2 | <Enter>%3 | <Space>%4")
                .arg(::QObject::tr("Select Item"))
                .arg(::QObject::tr("Change Field"))
                .arg(::QObject::tr("Confirm"))
                .arg(::QObject::tr("Select"));
    }

public slots:
    void abort();

signals:
    void finish();
    void error();
    void update(const QString &);

protected:
    virtual bool handle() = 0;


protected:
    FrameInterface* m_next;
    QVector<FrameInterface*> m_childFrame;
    FrameInterface* m_parent;
    FrameInterfacePrivate* m_private;
    FRAME_STATE m_currState;
    int m_currTask;
    bool m_cangetchild;

};


}  // namespace installer

#endif  // FRAMEINTERFACE_H
