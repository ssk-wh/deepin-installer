#ifndef PARTITION_LOG_FRAME_H
#define PARTITION_LOG_FRAME_H

#include "ui/interfaces_cli/frameinterfaceprivate.h"
#include "ui/interfaces_cli/frameinterface.h"
#include "ui/ncurses_widgets/ncurses_label.h"

namespace installer {

class PartitionLogFrame;
class PartitionLogFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
    friend PartitionLogFrame;
public:
    PartitionLogFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {
        initUI();
        initConnection();
        //updateTs();
    }

    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
private:
    NcursesLabel* m_label_title = nullptr;
};

class PartitionLogFrame : public FrameInterface
{
    Q_OBJECT
public:
    PartitionLogFrame(FrameInterface* parent);
    virtual ~PartitionLogFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;


protected:
    bool handle() override;
private:
    Q_DECLARE_PRIVATE_D(m_private, PartitionLogFrame)
};


}



#endif
