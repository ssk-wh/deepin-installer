#include "savelog_frame.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/delegates/main_window_util.h"
#include "base/file_util.h"
#include "service/log_manager.h"
#include "../../../base/command.h"
#include <QDateTime>
#include <QThread>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>

namespace installer {

SaveLogFramePrivate::SaveLogFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_isshow(false)
{
    initUI();
    initConnection();
}

SaveLogFramePrivate::~SaveLogFramePrivate()
{

}

void SaveLogFramePrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->dialog_attr());
    this->drawShadow(true);
    this->box();

    QString strBack = ::QObject::tr("Cancel");
    QString strNext = ::QObject::tr("Save Log");

    m_label_title = new NcursesLabel(this, 1, 1, begy(), begx());
    m_label_title->setFocusEnabled(false);
    m_label_title->hide();

    m_partitionlist = new NcursesListView(this, (height() - 3) / 2, width() / 2, begy() + 4, begx() + (width() / 3));

    m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
    m_pBackButton->drawShadow(true);
    m_pBackButton->box();
    m_pBackButton->setObjectName(strBack);
    m_pBackButton->hide();

    m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);
    m_pNextButton->drawShadow(true);
    m_pNextButton->box();
    m_pNextButton->setObjectName(strNext);
    m_pNextButton->setFocus(true);

    m_diskManager = new DDiskManager;
    m_diskManager->setWatchChanges(false);
    connect(m_diskManager, &DDiskManager::blockDeviceAdded, this, &SaveLogFramePrivate::onBlockDeviceAdded, Qt::QueuedConnection);
    connect(m_diskManager, &DDiskManager::blockDeviceRemoved, this, &SaveLogFramePrivate::onBlockDeviceRemoved, Qt::QueuedConnection);
    connect(m_diskManager, &DDiskManager::diskDeviceRemoved, this, &SaveLogFramePrivate::onDeviceRemoved, Qt::QueuedConnection);

    connect(this, SIGNAL(signal_AddDeviceParth(QString)), this, SLOT(slot_AddDeviceParth(QString)));
}

void SaveLogFramePrivate::layout()
{
    int beginY = begy();
    QString strBack = m_pBackButton->text();
    QString strNext = m_pNextButton->text();

    m_label_title->adjustSizeByContext();
    m_label_title->mvwin(beginY + 2, begx() + 1);

    m_pBackButton->erase();
    m_pBackButton->resetBackground();
    m_pBackButton->box(ACS_VLINE,ACS_HLINE);
    m_pBackButton->setText(strBack);
    m_pBackButton->show();

    m_pNextButton->erase();
    m_pNextButton->resetBackground();
    m_pNextButton->box(ACS_VLINE,ACS_HLINE);
    m_pNextButton->setText(strNext);
    m_pNextButton->show();
}

void SaveLogFramePrivate::updateTs()
{
    printTitle(::QObject::tr("Save Log"), width());
    m_label_title->setText("    " + ::QObject::tr("Save the error log to a removable device"));

    QString strBack = ::QObject::tr("Cancel");
    QString strNext = ::QObject::tr("Save Log");
    m_pBackButton->setText(strBack);
    m_pNextButton->setText(strNext);

    layout();
}

void SaveLogFramePrivate::initConnection()
{
    connect(m_pBackButton, &NcursesButton::clicked, this, &SaveLogFramePrivate::doBackBtnClicked);
    connect(m_pNextButton, &NcursesButton::clicked, this, &SaveLogFramePrivate::doNextBtnClicked);
}

bool SaveLogFramePrivate::validate()
{

}

void SaveLogFramePrivate::show()
{
    if(!m_isshow) {
        m_diskManager->setWatchChanges(true);
        NCursesWindowBase::show();
        m_isshow = true;
    }
}

void SaveLogFramePrivate::hide()
{
    m_diskManager->setWatchChanges(false);
    NCursesWindowBase::hide();
    m_isshow = false;
}

void SaveLogFramePrivate::keyPresseEvent(int keycode)
{
    if(!m_isshow) {
        return;
    } else {
        FrameInterfacePrivate::keyEventTriger(keycode);
    }
}

void SaveLogFramePrivate::doBackBtnClicked()
{
    emit backToPreviousPage();
}

void SaveLogFramePrivate::doNextBtnClicked()
{
    const QString& logPath {
        QString("%1/deepin-installer.%2.log")
                .arg(m_diskpaths.at(m_partitionlist->getCurrentIndex()).second)
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss"))
    };

    qDebug() << "Log file: " << logPath;

    const QString current_log_file(GetLogFilepath());
    if (!CopyFile(current_log_file, logPath, true)) {
      qCritical() << "Failed to copy log to:" << logPath;
    }

    emit backToPreviousPage();
}

void SaveLogFramePrivate::onBlockDeviceAdded(const QString &path)
{
    //emit signal_AddDeviceParth(path);
    QSharedPointer<DBlockDevice> device(DDiskManager::createBlockDevice(path));
    if (!device->readOnly() && device->hasPartition()) {
        //try mount to check writable
        auto checkWritable = [=](const QString& mount) -> bool {
            return QFileInfo(mount).isWritable();
        };

        QString testlistitem = QString("%1 %2 %3Mib %4").arg(QString(device->device()))
                                  .arg(device->idType())
                                  .arg(device->size() / (1024*1024))
                                  .arg(device->idLabel());
       if(!m_pathlist.contains(testlistitem)){
            m_pathlist.append(testlistitem);
       }

        const QByteArrayList& mountPoints = device->mountPoints();
        if (!mountPoints.isEmpty()) {
            for (const QByteArray& array : mountPoints) {
                if (checkWritable(array)) {
                    m_diskpaths.append(QPair<QString, QString>(path, array));
                }
            }
        } else {
            QString testpath = path.right(path.length() - path.lastIndexOf("/") - 1);
            SpawnCmd("mkdir", {QString("/mnt/%1").arg(testpath)});
            SpawnCmd("mount", {QString("/dev/%1").arg(testpath), QString("/mnt/%1").arg(testpath)});
            m_diskpaths.append(QPair<QString, QString>(path, QString("/mnt/%1").arg(testpath)));
        }

        m_partitionlist->setList(m_pathlist);
        m_partitionlist->show();
    }

    //QThread::msleep(3000);
}

void SaveLogFramePrivate::onBlockDeviceRemoved(const QString &path)
{
    QString testpath = path.right(path.length() - path.lastIndexOf("/") - 1);
    for (QStringList::iterator itera = m_pathlist.begin(); itera != m_pathlist.end(); ) {
        if ((*itera).contains(testpath)) {
            itera = m_pathlist.erase(itera);
        } else {
            itera++;
        }
    }
    for (QVector<QPair<QString, QString>>::iterator itera = m_diskpaths.begin(); itera != m_diskpaths.end(); ) {
        if ((*itera).first.contains(path)) {
            SpawnCmd("umount", {QString("/mnt/%1").arg(testpath)});
            SpawnCmd("rm", {"-r", QString("/mnt/%1").arg(testpath)});
            itera = m_diskpaths.erase(itera);
        } else {
            itera++;
        }
    }

    m_partitionlist->setList(m_pathlist);
    m_partitionlist->show();
}

void SaveLogFramePrivate::onDeviceRemoved(const QString &path)
{

}

void SaveLogFramePrivate::slot_AddDeviceParth(QString testpath)
{
    QSharedPointer<DBlockDevice> device(DDiskManager::createBlockDevice(testpath));
    if (!device->readOnly() && device->hasPartition()) {
        //try mount to check writable
        auto checkWritable = [=](const QString& mount) -> bool {
            return QFileInfo(mount).isWritable();
        };

        const QByteArrayList& mountPoints = device->mountPoints();
        if (!mountPoints.isEmpty()) {
            for (const QByteArray& array : mountPoints) {
                if (checkWritable(array)) {
                    m_pathlist.append(QString("%1 %2 %3Mib %4")
                                      .arg(QString(device->device()))
                                      .arg(device->idType())
                                      .arg(device->size() / (1024*1024))
                                      .arg(device->idLabel()));
                    m_diskpaths.append(QPair<QString, QString>(testpath, array));
                }
            }
        }

        m_partitionlist->setList(m_pathlist);
        m_partitionlist->show();
    }
}

SaveLogFrame::SaveLogFrame(FrameInterface *parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new SaveLogFramePrivate(nullptr, h, w, beginY, beginX);

    Q_D(SaveLogFrame);
    connect(parent->getPrivate(), SIGNAL(keyEventTrigerSignal(int)), d, SLOT(keyPresseEvent(int)));
    connect(d, &SaveLogFramePrivate::backToPreviousPage, [parent](){
        parent->hideAllChild();
        parent->show();
    });
}

SaveLogFrame::~SaveLogFrame()
{

}

bool SaveLogFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        //readConf();
        m_private->layout();
    }
    return true;
}

QString SaveLogFrame::getFrameName()
{
    return "";
}

bool SaveLogFrame::handle()
{
    return true;
}


}
