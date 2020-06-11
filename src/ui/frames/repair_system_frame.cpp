/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "repair_system_frame.h"
#include "ui/interfaces/frameinterfaceprivate.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "partman/os_prober.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/component_widget.h"
#include "ui/widgets/operator_widget.h"
#include "ui/utils/widget_util.h"


#include <QProcess>
#include <QDebug>
#include <QTranslator>
#include <QApplication>

DWIDGET_USE_NAMESPACE

namespace  {
    const int kItemWidth = 660;
    const int kItemHeight = 100;
    const char kLanguageFileTpl[] = I18N_DIR "/deepin-installer-zh_CN.qm";
}

namespace installer {

class RepairSystemFramePrivate : public FrameInterfacePrivate {
    Q_OBJECT
public:
    explicit RepairSystemFramePrivate(FrameInterface* parent):
        FrameInterfacePrivate(parent),
        q_ptr(qobject_cast<RepairSystemFrame* >(parent))
    {
        this->updateTs();
        this->initUi();
        this->initConnection();
    }

protected:
    // Verify that jumping to the next frame is allowed.
    bool validate() const;

private:
    void initUi();
    void initConnection();

    void setupTs();
    void updateTs();
    void removeTs() const;

private:
    RepairSystemFrame* q_ptr = nullptr;
    Q_DECLARE_PUBLIC(RepairSystemFrame)
    std::list<std::pair<std::function<void (QString)>, QString>> m_trList;

    TitleLabel*     m_titleLabel = nullptr;
    CommentLabel*   m_commentLabel = nullptr;
    OperatorWidget* m_installerWidget = nullptr;
    OperatorWidget* m_repairWidget = nullptr;
    QTranslator*    m_trans = nullptr;
};

}

bool installer::RepairSystemFramePrivate::validate() const
{
    this->removeTs();
    return true;
}

void installer::RepairSystemFramePrivate::initUi() {
    m_titleLabel = new TitleLabel();
    m_commentLabel= new CommentLabel();

    m_installerWidget = new OperatorWidget;
    m_installerWidget->setFixedSize(kItemWidth, kItemHeight);
    m_installerWidget->setSelectIcon(":/images/select_blue.svg");

    m_repairWidget = new OperatorWidget;
    m_repairWidget->setSelect(true);
    m_repairWidget->setFixedSize(kItemWidth, kItemHeight);
    m_repairWidget->setSelectIcon(":/images/select_blue.svg");

    nextButton->setEnabled(false);

    centerLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    centerLayout->addWidget(m_commentLabel, 0, Qt::AlignHCenter| Qt::AlignTop);
    centerLayout->addStretch(2);
    centerLayout->addWidget(m_installerWidget, 0, Qt::AlignHCenter);
    centerLayout->addStretch(1);
    centerLayout->addWidget(m_repairWidget, 0, Qt::AlignHCenter);
    centerLayout->addStretch(25);
    centerLayout->addWidget(nextButton, 0, Qt::AlignHCenter | Qt::AlignBottom);
}

void installer::RepairSystemFramePrivate::initConnection() {
    connect(m_installerWidget, &OperatorWidget::clicked, this, [=] {
        m_repairWidget->setSelect(false);
        nextButton->setEnabled(true);
    });

    connect(m_repairWidget, &OperatorWidget::clicked, this, [=] {
        m_installerWidget->setSelect(false);
        nextButton->setEnabled(true);
    });

    disconnect(nextButton, nullptr, nullptr, nullptr);
    connect(nextButton, &QPushButton::clicked, this, [=] {
        if (m_installerWidget->isCheckable()) {
            q_ptr->m_proxy->hideChildFrame();
            q_ptr->m_proxy->nextFrame();
        } else if (m_repairWidget->isCheckable()) {
            q_ptr->repairSystem();
        }
    });
}

void installer::RepairSystemFramePrivate::setupTs()
{
    m_titleLabel->setText(::QObject::tr("System Setup"));
    m_commentLabel->setText(::QObject::tr("Choose an option for your system"));

    m_installerWidget->setTitle(::QObject::tr("Install"));
    m_installerWidget->setBody(::QObject::tr("Install the system in your installation media."));

    m_repairWidget->setTitle(::QObject::tr("Repair"));
    m_repairWidget->setBody(::QObject::tr("Use recovery tools to fix system issues."));

    nextButton->setText(::QObject::tr("Enter the"));
}

void installer::RepairSystemFramePrivate::updateTs()
{
    m_trans = new QTranslator(this);
    m_trans->load(kLanguageFileTpl);
    qApp->installTranslator(m_trans);
}

void installer::RepairSystemFramePrivate::removeTs() const
{
    if (m_trans != nullptr) {
        qApp->removeTranslator(m_trans);
    }
}

installer::RepairSystemFrame::RepairSystemFrame(installer::FrameProxyInterface *frameProxyInterface, QWidget *parent):
    FrameInterface(frameProxyInterface, parent),
    m_private(new RepairSystemFramePrivate(this))
{
    setObjectName("repair_system_frame");
    setFrameType(FrameType::Frame);
}

installer::RepairSystemFrame::~RepairSystemFrame()
{
}

void installer::RepairSystemFrame::init()
{

}

void installer::RepairSystemFrame::finished()
{

}

bool installer::RepairSystemFrame::shouldDisplay() const
{
    return isRepair() && !GetSettingsBool(kSkipRepairSystemPage);
}

QString installer::RepairSystemFrame::returnFrameName() const
{
    return ::QObject::tr("System Setup");
}

void installer::RepairSystemFrame::repairSystem() const
{
    QString repairScript = GetSettingsString(kRepairScriptPoints);
    if (!QFileInfo::exists(repairScript)) {
        repairScript = DATAS_DIR + repairScript;
    }

    QProcess process;
    process.setProgram(repairScript);
    if (!process.startDetached()) {
        qCritical() << QString("Repair script <%1> execution failure. ").arg(repairScript) << process.errorString();
    }
    Q_EMIT repair();
}

void installer::RepairSystemFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void installer::RepairSystemFrame::changeEvent(QEvent *event)
{
    Q_D(RepairSystemFrame);
    if (event->type() == QEvent::LanguageChange) {
        d->setupTs();
    }

    return FrameInterface::changeEvent(event);
}

bool installer::RepairSystemFrame::isRepair() const
{
#ifdef QT_DEBUG
    return true;
#endif // QT_DEBUG
    return !GetOsProberItems().isEmpty();
}

#include "repair_system_frame.moc"

