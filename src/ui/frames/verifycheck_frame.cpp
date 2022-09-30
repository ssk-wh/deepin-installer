//
// Created by dtx on 2022/9/26.
//
#include "ui/frames/verifycheck_frame.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include <DSpinner>
#include <QDebug>
#include <QVBoxLayout>
#include <QThread>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace installer {

class VerifyCheckFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

public:
    explicit VerifyCheckFramePrivate(FrameInterface* parent) : FrameInterfacePrivate(parent){}

    void initUI();
    void initConnect();
    void startCalculateTip(bool isStart);

    DSpinner *m_calculateTip = nullptr;
    QLabel *m_calculateTipLabel = nullptr;
};

void VerifyCheckFramePrivate::initUI()
{
    QVBoxLayout *spinnerLayout = new QVBoxLayout;
    m_calculateTip = new DSpinner;
    m_calculateTip->setFixedSize(128, 128);
    m_calculateTip->setFocusPolicy(Qt::NoFocus);
    m_calculateTipLabel = new QLabel;
    m_calculateTipLabel->setText(::QObject::tr("Verifying image files..."));
    spinnerLayout->addStretch();
    spinnerLayout->addWidget(m_calculateTip, 0, Qt::AlignHCenter);
    spinnerLayout->addWidget(m_calculateTipLabel, 0, Qt::AlignHCenter);
    spinnerLayout->addStretch();
    centerLayout->setSpacing(10);
    centerLayout->addLayout(spinnerLayout);

    nextButton->hide();
}

void VerifyCheckFramePrivate::initConnect()
{

}

void VerifyCheckFramePrivate::startCalculateTip(bool isStart)
{
    if (m_calculateTip != nullptr) {
        m_calculateTip->start();
    }
}

VerifyCheckFrame::VerifyCheckFrame(FrameProxyInterface* frameProxyInterface, QWidget *parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new VerifyCheckFramePrivate(this))
{

}


VerifyCheckFrame::~VerifyCheckFrame()
{

}

void VerifyCheckFrame::init()
{
    m_private->initUI();
}

void VerifyCheckFrame::finished()
{

}

bool VerifyCheckFrame::shouldDisplay() const
{
    return true;
}

QString VerifyCheckFrame::returnFrameName() const
{
    return  ::QObject::tr("Verify ISO");
}

void VerifyCheckFrame::verifyCheck()
{
    QThread* installThread = new QThread;
    connect(&verify, &Verify::done, this, [&](bool flag){
        Q_EMIT verifyDone(flag);
    }, Qt::QueuedConnection);
    connect(installThread, &QThread::finished, installThread, &QThread::deleteLater);
    connect(installThread, &QThread::started, &verify, &Verify::start);

    verify.moveToThread(installThread);  // 将对象move到线程中运行
    installThread->start();
    Q_EMIT verifyStart();
}

void VerifyCheckFrame::changeEvent(QEvent *event)
{
    return FrameInterface::changeEvent(event);
}

void VerifyCheckFrame::hideEvent(QHideEvent *event)
{
    m_private->startCalculateTip(false);
    return FrameInterface::hideEvent(event);
}

void VerifyCheckFrame::showEvent(QShowEvent *event)
{
    m_private->startCalculateTip(true);
    verifyCheck();
    return FrameInterface::showEvent(event);
}

bool VerifyCheckFrame::focusSwitch()
{
    return true;
}

bool VerifyCheckFrame::doSpace()
{
    return true;
}

bool VerifyCheckFrame::doSelect()
{
    return true;
}

bool VerifyCheckFrame::directionKey(int keyvalue)
{
    return true;
}

void Verify::start()
{
    bool flag = true;
    QStringList sourceFileList = GetSettingsStringList("DI_INITRD_SOURCE_PATH");
    QStringList verifyFileList = GetSettingsStringList("DI_INITRD_VERIFY_PATH");
    for (int i = 0; i < sourceFileList.size(); i++) {
        QString sourceFile = QString("%1/../%2").arg(GetOemDir().absolutePath(), sourceFileList.at(i));
        QString verifyFile = "";
        if (!verifyFileList.isEmpty()) {
            verifyFile = QString("%1/../%2").arg(GetOemDir().absolutePath(),
                                                 verifyFileList.at(i < verifyFileList.size() ? i : verifyFileList.size() - 1));
        }

        QString err;
        if (!handleVerify(sourceFile, verifyFile, err)) {
            qCritical() << "deepin squashfs verify: " << err;
            // 兼容安装器的调试模式
            if (!GetSettingsBool("system_debug")) {
                SetSettingString("DI_DEEPIN_VERIFY_STATUS", sourceFile);
                flag = false;
            }
        }
    }

    Q_EMIT done(flag);
}

}


#include "verifycheck_frame.moc"
