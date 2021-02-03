#ifndef INSTALL_SAVELOG_FRAME_CLI_H
#define INSTALL_SAVELOG_FRAME_CLI_H

#include "ui/interfaces_cli/frameinterface.h"

#include <QMap>
#include <QSharedPointer>

class DBlockDevice;
class DDiskDevice;
class DDiskManager;

namespace installer {

class Partition;
class NcursesListView;
class NcursesLabel;

class SaveLogFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    SaveLogFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    ~SaveLogFramePrivate();

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    void show() override;
    void hide() override;
    virtual void onKeyPress(int keycode) override;

signals:
    void backToPreviousPage();
    void signal_AddDeviceParth(QString testpath);

private slots:
    void keyPresseEvent(int keycode);
    void doBackBtnClicked();
    void doNextBtnClicked();
    void onBlockDeviceAdded(const QString& path);
    void onBlockDeviceRemoved(const QString& path);
    void onDeviceRemoved(const QString& path);
    void slot_AddDeviceParth(QString testpath);

private:
    bool m_isshow;
    NcursesLabel* m_label_title      = nullptr;
    NcursesListView* m_partitionlist = nullptr;
    //QSharedPointer<Partition> m_selectPartition;
    //QMap<QSharedPointer<Partition>, QSharedPointer<DBlockDevice>> m_deviceButtonMap;
    DDiskManager* m_diskManager;
    QStringList m_pathlist;
    QVector<QPair<QString, QString>> m_diskpaths;
    //QMap<QSharedPointer<DDiskDevice>, QList<QSharedPointer<DBlockDevice>>> m_deviceMap;
};

class SaveLogFrame : public FrameInterface
{
    Q_OBJECT
public:
    SaveLogFrame(FrameInterface* parent);
    virtual ~SaveLogFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

protected:
    bool handle() override;

private:
    Q_DECLARE_PRIVATE_D(m_private, SaveLogFrame)
};

}

#endif // INSTALL_SAVELOG_FRAME_CLI_H
