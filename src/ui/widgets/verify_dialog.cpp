#include "verify_dialog.h"
#include "service/settings_manager.h"

#include <QProgressBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QThread>

using namespace installer;

VerifyDialog::VerifyDialog(QWidget *parten):
    DDialog(parten)
{
    //    标题
    QLabel *title = new QLabel("Verify Dialog");
    title->setStyleSheet("font-size:24px; font-weight:bold;");
    QHBoxLayout *titleHboxLayout = new QHBoxLayout;
    titleHboxLayout->addStretch();
    titleHboxLayout->addWidget(title);
    titleHboxLayout->addStretch();
    QWidget *titleWidget = new QWidget;
    titleWidget->setLayout(titleHboxLayout);

    // 进度显示
    m_progressWidget = new QProgressBar(this);
    m_progressWidget->setValue(0);
    m_progressWidget->setAlignment(Qt::AlignCenter);  // 对齐方式
    m_progressWidget->setRange(0, 100);

    QHBoxLayout *progressLayout = new QHBoxLayout;
    progressLayout->setContentsMargins(50, 0, 50, 0);
    progressLayout->addWidget(m_progressWidget);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
    mainLayout->addWidget(titleWidget);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(progressLayout);
    mainLayout->addSpacing(0);

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);

    this->setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    this->addContent(mainWidget);
}

void VerifyDialog::setValue(int value)
{
    m_progressWidget->setValue(value);
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
