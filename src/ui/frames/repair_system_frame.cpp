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

#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "partman/os_prober.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/operator_widget.h"

#include <QProcess>
#include <QVBoxLayout>
#include <QTranslator>
#include <QApplication>

DWIDGET_USE_NAMESPACE


namespace  {
    const int kItemWidth = 660;
    const int kItemHeight = 100;
    const char kLanguageFileTpl[] = I18N_DIR "/repair-zh_CN.qm";
}


installer::RepairSystemFrame::RepairSystemFrame(QWidget *parent):
    QFrame(parent)
{
    updateTs();
    initUi();
    initConnection();
}

bool installer::RepairSystemFrame::shouldDisplay() const
{
    return isRepair() && !GetSettingsBool(kSkipRepairSystemPage);
}

void installer::RepairSystemFrame::repairSystem() const
{
    QString repairScript = GetSettingsString(kRepairScriptPoints);

    QProcess process;
    process.setProgram(repairScript);
    if (!process.startDetached()) {
        qCritical() << QString("Repair script <%1> execution failure. ").arg(repairScript) << process.errorString();
    }

    Q_EMIT repair();
}

void installer::RepairSystemFrame::initUi()
{
    QString tsTitle = tr("Operation Choice");
    QString tsComment = tr("Please select your will to the operation of the system");
    TitleLabel* titleLabel = new TitleLabel(tsTitle);
    CommentLabel* commentLabel = new CommentLabel(tsComment);

    m_installerWidget = new OperatorWidget;
    m_installerWidget->setFixedSize(kItemWidth, kItemHeight);
    m_installerWidget->setSelectIcon(":/images/select_blue.svg");
    m_installerWidget->setTitle(tr("Install System"));
    m_installerWidget->setBody(tr("Choose to install system, will be installed on the system in the storage medium."));

    m_repairWidget = new OperatorWidget;
    m_repairWidget->setFixedSize(kItemWidth, kItemHeight);
    m_repairWidget->setSelectIcon(":/images/select_blue.svg");
    m_repairWidget->setTitle(tr("Repair System"));
    m_repairWidget->setBody(tr("Choose to repair the system, will enter the live system to repair the original UOS system."));

    m_nextButton = new NavButton;
    m_nextButton->setText(tr("Enter the"));
    m_nextButton->setEnabled(false);

    QVBoxLayout* centerLayout = new QVBoxLayout;
    centerLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    centerLayout->addWidget(commentLabel, 0, Qt::AlignHCenter| Qt::AlignTop);
    centerLayout->addStretch(2);
    centerLayout->addWidget(m_installerWidget, 0, Qt::AlignHCenter);
    centerLayout->addStretch(1);
    centerLayout->addWidget(m_repairWidget, 0, Qt::AlignHCenter);
    centerLayout->addStretch(25);
    centerLayout->addWidget(m_nextButton, 0, Qt::AlignHCenter | Qt::AlignBottom);
    setLayout(centerLayout);
    setFocus();
}

void installer::RepairSystemFrame::initConnection() const
{
    connect(m_installerWidget, &OperatorWidget::clicked, this, [=] {
       m_repairWidget->setSelect(false);
       m_nextButton->setEnabled(true);
       m_nextButton->setFocus();
    });

    connect(m_repairWidget, &OperatorWidget::clicked, this, [=] {
       m_installerWidget->setSelect(false);
       m_nextButton->setEnabled(true);
       m_nextButton->setFocus();
    });

    m_repairWidget->click();

    disconnect(m_nextButton, nullptr, nullptr, nullptr);
    connect(m_nextButton, &QPushButton::clicked, this, [=] {
       if (m_installerWidget->isCheckable()) {
           Q_EMIT finished();
       } else if (m_repairWidget->isCheckable()) {
           repairSystem();
       }
    });
}

bool installer::RepairSystemFrame::isRepair() const
{
#ifdef QT_DEBUG
    return true;
#endif // QT_DEBUG
    return !GetOsProberItems().isEmpty();
}

void installer::RepairSystemFrame::updateTs()
{
    QTranslator* trans = new QTranslator(this);
    trans->load(kLanguageFileTpl);
    qApp->installTranslator(trans);
}
