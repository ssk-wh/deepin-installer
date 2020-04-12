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
#include <QComboBox>
#include <QStringListModel>
#include <memory>
#include <QTimer>

#include "service/settings_manager.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/line_edit.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/system_info_tip.h"

using namespace installer;

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
        setObjectName("WhiteBackgroundWidget");
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

        setStyleSheet("#WhiteBackgroundWidget { " + backgroundCSS + borderRadiusCSS + "}");
        setMinimumSize(400, 500);
    }
};

class NetworkDeviceWidget : public QFrame {
    Q_OBJECT
public:
    explicit NetworkDeviceWidget(QWidget* parent = nullptr) : QFrame(parent) {
        setObjectName("NetworkDeviceWidget");

        m_deviceName = new QLabel;

        QVBoxLayout* layout = new QVBoxLayout;
        layout->setMargin(10);
        layout->setSpacing(0);

        layout->addWidget(m_deviceName);

        setLayout(layout);

        setStyleSheet("#NetworkDeviceWidget {"
                      "background: rgba(255, 255, 255, 0.1);"
                      "border-radius: 5px;"
                      "margin: 2px;"
                      "}");
    }

    void setDeviceInfo(const QNetworkInterface& interface) {
        m_deviceName->setText(tr("Ethernet (%1)").arg(interface.humanReadableName()));
        m_interface = interface;
    }

    QNetworkInterface interface() const {
        return m_interface;
    }

signals:
    void clicked() const;

protected:
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked();

        return QFrame::mousePressEvent(event);
    }

private:
    QLabel* m_deviceName;
    QNetworkInterface m_interface;
};

class NetworkEditWidget : public QWidget
{
    Q_OBJECT
public:
    enum class DHCPTYpe {
        Auto = 0,
        Manual,
    };

    explicit NetworkEditWidget (QWidget* parent = nullptr) : QWidget(parent) {
        m_connectTypeWidget = new QWidget;
        m_dhcpTypeWidget = new QComboBox;
        m_ipWidget = new QWidget;
        m_maskWidget = new QWidget;
        m_gatewayWidget = new QWidget;
        m_primaryDNSWidget = new QWidget;
        m_secondDNSWidget = new QWidget;
        m_editBtn = new NavButton(tr("Edit"));
        m_acceptBtn = new NavButton(tr("Accept"));
        m_dhcpType = DHCPTYpe::Auto;

        m_ipv4Edit = new LineEdit(QString(":/images/hostname_12.svg"));
        m_maskEdit = new LineEdit(QString(":/images/hostname_12.svg"));
        m_gatewayEdit = new LineEdit(QString(":/images/hostname_12.svg"));
        m_primaryDNSEdit = new LineEdit(QString(":/images/hostname_12.svg"));
        m_secondDNSEdit = new LineEdit(QString(":/images/hostname_12.svg"));

        m_ipv4Edit->setPlaceholderText(tr("IP Address"));
        m_maskEdit->setPlaceholderText(tr("Netmask"));
        m_gatewayEdit->setPlaceholderText(tr("Gateway"));
        m_primaryDNSEdit->setPlaceholderText(tr("Primary DNS"));
        m_secondDNSEdit->setPlaceholderText(tr("Secondary DNS"));

        m_errorTip = new SystemInfoTip(this);
        m_errorTip->hide();

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->setMargin(0);
        mainLayout->setSpacing(20);

        QList<QWidget*> tmpL {
                    m_connectTypeWidget,
                    m_ipWidget,
                    m_maskWidget,
                    m_gatewayWidget,
                    m_primaryDNSWidget,
                    m_secondDNSWidget,
                    m_ipv4Edit,
                    m_maskEdit,
                    m_gatewayEdit,
                    m_primaryDNSEdit,
                    m_secondDNSEdit
        };

        QMap<QWidget*, QString> tmpM {
            {m_ipWidget, tr("Ip:")},
            {m_maskWidget, tr("Mask:")},
            {m_gatewayWidget, tr("Gateway:")},
            {m_primaryDNSWidget, tr("Primary DNS:")},
            {m_secondDNSWidget, tr("Second DNS:")},
        };

        for (auto it = tmpM.begin(); it != tmpM.end(); ++it) {
            QHBoxLayout* layout = new QHBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);

            QLabel* name = new QLabel(it.value());
            QLabel* valueName = new QLabel;

            layout->addWidget(name, 0, Qt::AlignLeft | Qt::AlignHCenter);
            layout->addWidget(valueName, 0, Qt::AlignRight | Qt::AlignHCenter);

            labelHandleMap[it.key()] = [=](const QString& text) -> void {
                valueName->setText(text);
            };

            labelTextMap[it.key()] = [=]() -> QString {
                return valueName->text();
            };

            it.key()->setLayout(layout);
        }

        QHBoxLayout* dhcpLayout = new QHBoxLayout;
        dhcpLayout->setMargin(0);
        dhcpLayout->setSpacing(0);

        QLabel* dhcpName = new QLabel("DHCP");
        dhcpLayout->addWidget(dhcpName, 0, Qt::AlignLeft | Qt::AlignHCenter);
        dhcpLayout->addWidget(m_dhcpTypeWidget, 0, Qt::AlignRight | Qt::AlignHCenter);

        QStringListModel* dhcpTypeModel = new QStringListModel(m_dhcpTypeWidget);
        dhcpTypeModel->setStringList({tr("Auto"), tr("Manual")});
        m_dhcpTypeWidget->setModel(dhcpTypeModel);

        m_dhcpTypeWidget->setFixedWidth(280);

        m_connectTypeWidget->setLayout(dhcpLayout);

        for (QWidget* w : tmpL) {
            mainLayout->addWidget(w);
        }

        m_widgetList = {
            {m_ipWidget, m_ipv4Edit},
            {m_maskWidget, m_maskEdit},
            {m_gatewayWidget, m_gatewayEdit},
            {m_primaryDNSWidget, m_primaryDNSEdit},
            {m_secondDNSWidget, m_secondDNSEdit},
        };

        for (auto it = m_widgetList.begin(); it != m_widgetList.end(); ++it) {
            it->second->hide();
            mainLayout->setAlignment(it->second, Qt::AlignHCenter);
        }

        m_validityCheck = std::unique_ptr<
            QRegularExpressionValidator>(new QRegularExpressionValidator(QRegularExpression(
            "((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?\\.){0,3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?")));

        std::list<LineEdit *> editList = { m_ipv4Edit, m_maskEdit, m_gatewayEdit,
                                           m_primaryDNSEdit, m_secondDNSEdit };

        for (auto it = editList.begin(); it != editList.end(); ++it) {
            (*it)->setValidator(m_validityCheck.get());
        }

        mainLayout->addStretch();
        mainLayout->addWidget(m_editBtn, 0, Qt::AlignHCenter);
        mainLayout->addWidget(m_acceptBtn, 0, Qt::AlignHCenter);

        setLayout(mainLayout);

        m_acceptBtn->hide();

        connect(m_ipv4Edit, &LineEdit::editingFinished, this, &NetworkEditWidget::checkIPValidity);
        connect(m_gatewayEdit, &LineEdit::editingFinished, this,
                &NetworkEditWidget::checkIPValidity);
        connect(m_primaryDNSEdit, &LineEdit::editingFinished, this,
                &NetworkEditWidget::checkIPValidity);
        connect(m_secondDNSEdit, &LineEdit::editingFinished, this,
                &NetworkEditWidget::checkIPValidity);
        connect(m_maskEdit, &LineEdit::editingFinished, this,
                &NetworkEditWidget::checkMaskValidity);

        m_editList = {
            m_ipv4Edit,
            m_maskEdit,
            m_gatewayEdit,
            m_primaryDNSEdit,
            m_secondDNSEdit
        };

        for (auto it = m_editList.begin(); it != m_editList.end(); ++it) {
            connect(*it, &LineEdit::textEdited, this, &NetworkEditWidget::onEditingLineEdit);
        }

        connect(m_editBtn, &QPushButton::clicked, this, &NetworkEditWidget::onEdit);
        connect(m_acceptBtn, &QPushButton::clicked, this, &NetworkEditWidget::onEditFinished);
        connect(m_dhcpTypeWidget, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
                , this, &NetworkEditWidget::onDHCPChanged);
    }

    void setIpConfig(const QNetworkAddressEntry& address) {
        labelHandleMap[m_ipWidget](address.ip().toString());
        labelHandleMap[m_maskWidget](address.netmask().toString());
        labelHandleMap[m_gatewayWidget](address.broadcast().toString());
        QFile resolv("/etc/resolv.conf");
        if (resolv.open(QIODevice::Text | QIODevice::ReadOnly)) {
            QStringList dnsList;
            QString line;
            QTextStream stream(&resolv);
            while (stream.readLineInto(&line)) {
                if (line.startsWith("nameserver")) {
                    dnsList << line.split(" ").last();
                }
            }

            if (!dnsList.isEmpty()) {
                labelHandleMap[m_primaryDNSWidget](dnsList.first());
                if (dnsList.length() > 1) {
                    labelHandleMap[m_secondDNSWidget](dnsList[1]);
                }
            }
        }
    }

    void onEdit() {
        for (auto it = m_widgetList.begin(); it != m_widgetList.end(); ++it) {
            it->first->hide();
            it->second->show();
        }
        m_editBtn->hide();
        m_acceptBtn->show();
        m_dhcpTypeWidget->setCurrentIndex(1);

        m_ipv4Edit->setText(labelTextMap[m_ipWidget]());
        m_maskEdit->setText(labelTextMap[m_maskWidget]());
        m_gatewayEdit->setText(labelTextMap[m_gatewayWidget]());
        m_primaryDNSEdit->setText(labelTextMap[m_primaryDNSWidget]());
        m_secondDNSEdit->setText(labelTextMap[m_secondDNSWidget]());
        m_dhcpTypeWidget->setEnabled(false);
    }

    void onEditFinished() {
        for (auto it = m_widgetList.begin(); it != m_widgetList.end(); ++it) {
            it->first->show();
            it->second->hide();
            labelHandleMap[it->first](it->second->text());
        }
        m_editBtn->show();
        m_acceptBtn->hide();
        m_dhcpTypeWidget->setEnabled(true);
        m_errorTip->hide();
    }

    void checkIPValidity()
    {
        LineEdit *edit = qobject_cast<LineEdit *>(sender());

        checkEditIPValidity(edit);
    }

    bool checkEditIPValidity(LineEdit *edit)
    {
        if (!checkip(edit->text())) {
            m_errorTip->setText(tr("IP address error: illegal IP address, please have a check."));
            m_errorTip->showBottom(edit);
            return false;
        }
        else {
            m_errorTip->hide();
            return true;
        }
    }

    bool checkMaskValidity()
    {
        if (!checkMask(m_maskEdit->text())) {
            m_errorTip->setText(tr("Netmask error: illegal netmask, please have a check."));
            m_errorTip->showBottom(m_maskEdit);
            return false;
        }
        else {
            m_errorTip->hide();
            return true;
        }
    }

    bool validate()
    {
        for (auto it = m_editList.begin(); it != m_editList.end(); ++it) {
            if (*it != m_maskEdit) {
                if (!checkEditIPValidity(*it)) {
                    return false;
                }
            }
            else {
                if (!checkMaskValidity()) {
                    return false;
                }
            }
        }

        return true;
    }

    // Hide error tip frame when line-edit is being edited.
    void onEditingLineEdit()
    {
        if (m_errorTip->isVisible()) {
            m_errorTip->hide();
        }
    }

    void onDHCPChanged(int index) {
        m_dhcpType = index == 0 ? DHCPTYpe::Auto : DHCPTYpe::Manual;

        if (m_dhcpType == DHCPTYpe::Manual) {
            onEdit();
        }
        else {
            onEditFinished();
        }
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange) {
            m_ipv4Edit->setPlaceholderText(tr("IP Address"));
            m_maskEdit->setPlaceholderText(tr("Netmask"));
            m_gatewayEdit->setPlaceholderText(tr("Gateway"));
            m_primaryDNSEdit->setPlaceholderText(tr("Primary DNS"));
            m_secondDNSEdit->setPlaceholderText(tr("Secondary DNS"));
            m_errorTip->hide();
        }

        return QWidget::event(event);
    }

    void setInterface(const QNetworkInterface& interface) {
        m_interface = interface;
    }

    QNetworkInterface interface() const {
        return m_interface;
    }

    QString ip() const {
        return labelTextMap[m_ipWidget]();
    }

    QString mask() const {
        return labelTextMap[m_maskWidget]();
    }

    QString gateway() const {
        return labelTextMap[m_gatewayWidget]();
    }

    QString primaryDNS() const {
        return labelTextMap[m_primaryDNSWidget]();
    }

    QString secondDNS() const {
        return labelTextMap[m_secondDNSWidget]();
    }

    DHCPTYpe connectType() const {
        return m_dhcpType;
    }

private:
    QWidget* m_connectTypeWidget;
    QWidget* m_ipWidget;
    QWidget* m_maskWidget;
    QWidget* m_gatewayWidget;
    QWidget* m_primaryDNSWidget;
    QWidget* m_secondDNSWidget;
    QList<QPair<QWidget*, LineEdit*>> m_widgetList;
    QList<LineEdit *> m_editList;
    NavButton* m_editBtn;
    NavButton* m_acceptBtn;
    LineEdit* m_ipv4Edit;
    LineEdit* m_maskEdit;
    LineEdit* m_gatewayEdit;
    LineEdit* m_primaryDNSEdit;
    LineEdit* m_secondDNSEdit;
    DHCPTYpe m_dhcpType;
    std::unique_ptr<QRegularExpressionValidator> m_validityCheck;
    SystemInfoTip*                               m_errorTip;
    QMap<QWidget*, std::function<void (const QString& text)>> labelHandleMap;
    QMap<QWidget*, std::function<QString ()>> labelTextMap;
    QComboBox *m_dhcpTypeWidget;
    QNetworkInterface m_interface;
};
}

NetworkFrame::NetworkFrame(QWidget *parent)
    : QWidget(parent)
    , m_skipButton(new NavButton(tr("Skip")))
    , m_saveButton(new NavButton(tr("Next")))
{
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

    m_currentNetworkEditWidget = new NetworkEditWidget;
    rightLayout->addWidget(m_currentNetworkEditWidget);

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

    rightLayout->addStretch();

    setStyleSheet("QLabel{color: white;}");

    connect(m_skipButton, &NavButton::clicked, this, &NetworkFrame::requestNext);
    connect(m_saveButton, &NavButton::clicked, this, &NetworkFrame::saveConf);

    const auto interfaces = QNetworkInterface::allInterfaces();
    QList<QNetworkInterface> interfaceList;
    for (const QNetworkInterface &i : interfaces) {
        // FIXME: name == lo
        if (i.name() != "lo" && i.flags().testFlag(QNetworkInterface::IsUp)) {
            interfaceList << i;
            NetworkDeviceWidget* device = new NetworkDeviceWidget;
            leftLayout->addWidget(device);
            device->setFixedHeight(100);
            device->setDeviceInfo(i);
            connect(device, &NetworkDeviceWidget::clicked, this, &NetworkFrame::onDeviceSelected);
        }
    }

    if (!interfaceList.isEmpty()) {
        m_currentNetworkEditWidget->setInterface(interfaceList.first());
        if (!interfaceList.first().addressEntries().isEmpty()) {
            m_currentNetworkEditWidget->setIpConfig(interfaceList.first().addressEntries().first());
        }
    }

    leftLayout->addStretch();
}

bool NetworkFrame::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        m_subTitle->setText(tr("Configure Network"));
        m_skipButton->setText(tr("Skip"));
        m_saveButton->setText(tr("Next"));
    }

    return QWidget::event(event);
}

void NetworkFrame::saveConf()
{
    if (!m_currentNetworkEditWidget->validate()) {
        return;
    }

    if (m_currentNetworkEditWidget->connectType() == NetworkEditWidget::DHCPTYpe::Manual) {
        const QString& ip = m_currentNetworkEditWidget->ip();
        const QString& mask = m_currentNetworkEditWidget->mask();
        const QString& gateway = m_currentNetworkEditWidget->gateway();
        const QString& primaryDNS = m_currentNetworkEditWidget->primaryDNS();
        const QString& secondDNS = m_currentNetworkEditWidget->secondDNS();
        const QString& interface = m_currentNetworkEditWidget->interface().name();

        QStringList cmd = QStringList() << "con"
                                        << "mod"
                                        << interface
                                        << "ipv4.addr"
                                        << QString("%1/%2").arg(ip).arg(coverMask(mask))
                                        << "gw4"
                                        << gateway;
        qDebug() << QProcess::execute("nmcli", cmd);

        qDebug() << QProcess::execute("nmcli", QStringList() << "con"
                                      << "mod"
                                      << interface
                                      << "ipv4.dns"
                                      << QString("%1 %2").arg(primaryDNS, secondDNS));

        qDebug() << QProcess::execute("nmcli", QStringList() << "con"
                                      << "up"
                                      << interface
                                      << "ifname"
                                      << interface);

    }

    emit requestNext();
}

void NetworkFrame::onDeviceSelected()
{
    NetworkDeviceWidget* device = qobject_cast<NetworkDeviceWidget*>(sender());
    if (!device->interface().addressEntries().isEmpty()) {
        m_currentNetworkEditWidget->setIpConfig(device->interface().addressEntries().first());
    }
    m_currentNetworkEditWidget->setInterface(device->interface());
}

#include "networkframe.moc"
