#include "networkframe.h"

#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QUuid>
#include <QVBoxLayout>
#include <list>
#include <utility>
#include <QEvent>
#include <QNetworkInterface>
#include <QProcess>
#include <DBackgroundGroup>

#include <networkmanagerqt/connection.h>
#include <networkmanagerqt/connectionsettings.h>
#include <networkmanagerqt/activeconnection.h>
#include <networkmanagerqt/manager.h>

#include "service/settings_manager.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/line_edit.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/system_info_tip.h"

using namespace installer;

namespace installer {
class WhiteBackgroundWidget : public QFrame {
    Q_OBJECT
public:
    enum class Position {
        Left,
        Center,
        Right,
        Top,
        Bottom
    };

    explicit WhiteBackgroundWidget(Position position, QWidget* parent = nullptr) : QFrame(parent) {
        const QString backgroundCSS("background: rgba(255, 255, 255, 0.1);");
        QString borderRadiusCSS("border-top-left-radius: %1;"
                                "border-top-right-radius: %2;"
                                "border-bottom-left-radius: %3;"
                                "border-bottom-right-radius: %4;");
        const QString radius = "5";
        const QString nonRadius = "0";
        switch (position) {
        case Position::Left: {
            borderRadiusCSS = borderRadiusCSS.arg(radius).arg(nonRadius).arg(radius).arg(nonRadius);
        }
            break;
        case Position::Right: {
            borderRadiusCSS = borderRadiusCSS.arg(nonRadius).arg(radius).arg(nonRadius).arg(radius);
        }
            break;
        case Position::Top: {
            borderRadiusCSS = borderRadiusCSS.arg(radius).arg(radius).arg(nonRadius).arg(nonRadius);
        }
            break;
        case Position::Bottom: {
            borderRadiusCSS = borderRadiusCSS.arg(nonRadius).arg(nonRadius).arg(radius).arg(radius);
        }
            break;
        default: {
            borderRadiusCSS = "";
        }
        }

        setStyleSheet(backgroundCSS + borderRadiusCSS);
        setMinimumSize(400, 500);
    }
};
}

static uint coverMask(const QString &source)
{
    std::list<char>   mask;
    const QStringList list = source.split(".");

    for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
        const QString &num   = *it;
        QByteArray     array = QString::number(num.toUInt(), 2).toUtf8();
        for (char c : array) {
            mask.push_back(c);
        }
    }

    uint m = 0;
    for (char c : mask) {
        m += QString(c).toUInt();
    }

    return m;
}

static bool checkip(QString ip)
{
    QRegularExpression rx(
        "^(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.("
        "\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$");
    return rx.match(ip).hasMatch();
}

static bool checkMask(QString mask)
{
    QRegularExpression rx(
        "^(254|252|248|240|224|192|128|0)\\.0\\.0\\.0|255\\.(254|252|248|240|224|192|128|"
        "0)\\.0\\.0|255\\.255\\.(254|252|248|240|224|192|128|0)\\.0|255\\.255\\.255\\.("
        "254|252|248|240|224|192|128|0)$");

    return rx.match(mask).hasMatch();
}

NetworkFrame::NetworkFrame(QWidget *parent)
    : QWidget(parent)
    , m_ipv4Edit(new LineEdit(QString(":/images/hostname_12.svg")))
    , m_maskEdit(new LineEdit(QString(":/images/hostname_12.svg")))
    , m_gatewayEdit(new LineEdit(QString(":/images/hostname_12.svg")))
    , m_primaryDNSEdit(new LineEdit(QString(":/images/hostname_12.svg")))
    , m_secondDNSEdit(new LineEdit(QString(":/images/hostname_12.svg")))
    , m_skipButton(new NavButton(tr("Skip")))
    , m_saveButton(new NavButton(tr("Next")))
{
    auto i = NetworkManager::activeConnections();
    for (auto t : i) {
        qDebug() << t->path();
    }

    m_validityCheck = std::unique_ptr<
        QRegularExpressionValidator>(new QRegularExpressionValidator(QRegularExpression(
        "((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?\\.){0,3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?")));

    std::list<LineEdit *> editList = { m_ipv4Edit, m_maskEdit, m_gatewayEdit,
                                       m_primaryDNSEdit, m_secondDNSEdit };

    for (auto it = editList.begin(); it != editList.end(); ++it) {
        (*it)->setValidator(m_validityCheck.get());
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(10);

    QLabel *logo_label = new QLabel;
    logo_label->setPixmap(installer::renderPixmap(GetVendorLogo()));

    layout->addWidget(logo_label, 0, Qt::AlignHCenter);

    m_subTitle = new QLabel(tr("Configure Network"));
    layout->addWidget(m_subTitle, 0, Qt::AlignHCenter);

    // 左侧布局
    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    WhiteBackgroundWidget* leftWidget = new WhiteBackgroundWidget(WhiteBackgroundWidget::Position::Left);
    leftWidget->setLayout(leftLayout);

    // 右侧布局
    QVBoxLayout* rightLayout = new QVBoxLayout;
    leftLayout->setMargin(0);
    leftLayout->setSpacing(5);
    WhiteBackgroundWidget* rightWidget = new WhiteBackgroundWidget(WhiteBackgroundWidget::Position::Right);
    rightWidget->setLayout(rightLayout);

    QHBoxLayout* centerLayout = new QHBoxLayout;
    centerLayout->setMargin(0);
    centerLayout->setSpacing(0);

    centerLayout->addStretch();
    centerLayout->addWidget(leftWidget);
    centerLayout->addSpacing(1);
    centerLayout->addWidget(rightWidget);
    centerLayout->addStretch();

    layout->addStretch();
    layout->addLayout(centerLayout);
    layout->addStretch();

    layout->addWidget(m_skipButton, 0, Qt::AlignHCenter);
    layout->addWidget(m_saveButton, 0, Qt::AlignHCenter);

    setLayout(layout);

    m_errorTip = new SystemInfoTip(rightWidget);

    for (auto it = editList.begin(); it != editList.end(); ++it) {
        rightLayout->addWidget((*it), 0, Qt::AlignHCenter);
    }

    rightLayout->addStretch();

    setStyleSheet("QLabel{color: white;}");

    connect(m_skipButton, &NavButton::clicked, this, &NetworkFrame::requestNext);
    connect(m_saveButton, &NavButton::clicked, this, &NetworkFrame::saveConf);
    connect(m_ipv4Edit, &LineEdit::editingFinished, this, &NetworkFrame::checkIPValidity);
    connect(m_gatewayEdit, &LineEdit::editingFinished, this,
            &NetworkFrame::checkIPValidity);
    connect(m_primaryDNSEdit, &LineEdit::editingFinished, this,
            &NetworkFrame::checkIPValidity);
    connect(m_secondDNSEdit, &LineEdit::editingFinished, this,
            &NetworkFrame::checkIPValidity);
    connect(m_maskEdit, &LineEdit::editingFinished, this,
            &NetworkFrame::checkMaskValidity);
}

bool NetworkFrame::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        m_subTitle->setText(tr("Configure Network"));
        m_ipv4Edit->setPlaceholderText(tr("IP Address"));
        m_maskEdit->setPlaceholderText(tr("Netmask"));
        m_gatewayEdit->setPlaceholderText(tr("Gateway"));
        m_primaryDNSEdit->setPlaceholderText(tr("Primary DNS"));
        m_secondDNSEdit->setPlaceholderText(tr("Secondary DNS"));
        m_skipButton->setText(tr("Skip"));
        m_saveButton->setText(tr("Next"));
        m_errorTip->hide();
    }

    return QWidget::event(event);
}

void NetworkFrame::saveConf()
{
    std::list<LineEdit *> editList = { m_ipv4Edit, m_gatewayEdit, m_primaryDNSEdit,
                                       m_secondDNSEdit };

    for (LineEdit *edit : editList) {
        if (!checkip(edit->text())) {
            emit edit->editingFinished();
            return;
        }
    }

    if (!checkMask(m_maskEdit->text())) {
        emit m_maskEdit->editingFinished();
        return;
    }

    const auto interfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface interface;
    for (const QNetworkInterface i : interfaces) {
        // FIXME: name == lo
        if (i.name() != "lo") {
            interface = i;
            break;
        }
    }

    qDebug() << QProcess::execute("nmcli", QStringList() << "con"
                                                         << "add"
                                                         << "type"
                                                         << "ethernet"
                                                         << "con-name"
                                                         << QString("\"%1-lab\"").arg(interface.name())
                                                         << "ifname"
                                                         << interface.name()
                                                         << "ip4"
                                                         << QString("%1/%2").arg(m_ipv4Edit->text()).arg(coverMask(m_maskEdit->text()))
                                                         << "gw4"
                                                         << m_gatewayEdit->text()
                                                         );

    qDebug() << QProcess::execute("nmcli", QStringList() << "con"
                                                         << "mod"
                                                         << QString("\"%1-lab\"").arg(interface.name())
                                                         << "ipv4.dns"
                                                         << QString("%1 %2").arg(m_primaryDNSEdit->text(), m_secondDNSEdit->text()));

    qDebug() << QProcess::execute("nmcli", QStringList() << "con"
                                                         << "up"
                                                         << QString("\"%1-lab\"").arg(interface.name())
                                                         << "ifname"
                                                         << interface.name());

    emit requestNext();
}

void NetworkFrame::checkIPValidity()
{
    LineEdit *edit = qobject_cast<LineEdit *>(sender());

    if (!checkip(edit->text())) {
        m_errorTip->setText(tr("IP address error: illegal IP address, please have a check."));
        m_errorTip->showBottom(edit);
    }
    else {
        m_errorTip->hide();
    }
}

void NetworkFrame::checkMaskValidity()
{
    if (!checkMask(m_maskEdit->text())) {
        m_errorTip->setText(tr("Netmask error: illegal netmask, please have a check."));
        m_errorTip->showBottom(m_maskEdit);
    }
    else {
        m_errorTip->hide();
    }
}

#include "networkframe.moc"
