#include "control_platform_frame.h"

#include "ui/interfaces/frameinterfaceprivate.h"

#include "ui/widgets/line_edit.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/table_combo_box.h"
#include "service/settings_manager.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/comment_label.h"
#include "service/settings_name.h"

#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QVBoxLayout>
#include <QtNetwork>
#include <QLabel>
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDBusInterface>
#include <DSysInfo>

using namespace installer;

namespace installer {
class ControlPlatformFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

    friend ControlPlatformFrame;

public:
    explicit ControlPlatformFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , m_titleLbl(new TitleLabel(::QObject::tr("Set Control Region")))
        , m_subTitleLbl(new CommentLabel(::QObject::tr("Set the region for UOS EndPoint Management Platform")))
        , m_serverLineEdit(new LineEdit(QString(":/images/hostname_12.svg")))
        , m_regionBox(new TableComboBox)
        , m_regionModel(new ControlPlatformRegionModel(this))
        , m_macInfoLayout(new QVBoxLayout)
        , m_ipInfoLayout(new QVBoxLayout)
        , q_ptr(qobject_cast<ControlPlatformFrame* >(parent))
    {}

    TitleLabel*                 m_titleLbl = nullptr;
    CommentLabel*               m_subTitleLbl = nullptr;
    LineEdit*                   m_serverLineEdit = nullptr;
    TableComboBox*              m_regionBox = nullptr;
    ControlPlatformRegionModel* m_regionModel = nullptr;
    QUrl                        m_serverUrl;
    QList<RegionInfo>           m_regionInfo;
    QVBoxLayout*                m_macInfoLayout = nullptr;
    QVBoxLayout*                m_ipInfoLayout = nullptr;
    QNetworkAccessManager*      networkAccessManager = nullptr;
    ControlPlatformFrame*       q_ptr = nullptr;

    void initUI();
    void initConnection();
    void onNetworkFinished(QNetworkReply* reply);
    void onRegionSelected();
    void onNetworkStateChanged();
};

void ControlPlatformFramePrivate::initUI()
{
    QDBusInterface* iface = new QDBusInterface(
        "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);

    if (iface->isValid()) {
        connect(iface, SIGNAL(StateChanged(uint)), this, SLOT(onNetworkStateChanged()));
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(10);

    m_regionBox->setHeight(300);
    m_regionBox->setModel(m_regionModel);

    QLabel *logo_label = new QLabel;
    logo_label->setPixmap(installer::renderPixmap(GetVendorLogo()));

    layout->addWidget(logo_label, 0, Qt::AlignHCenter);
    layout->addWidget(m_titleLbl, 0, Qt::AlignHCenter);
    layout->addWidget(m_subTitleLbl, 0, Qt::AlignHCenter);

    m_titleLbl->setObjectName("TitleLabel");
    m_subTitleLbl->setObjectName("SubTitleLabel");

    layout->addStretch();

    QHBoxLayout* ipInfo = new QHBoxLayout;
    ipInfo->setMargin(0);
    ipInfo->setSpacing(0);

    ipInfo->addStretch();
    ipInfo->addLayout(m_macInfoLayout);
    ipInfo->addSpacing(30);
    ipInfo->addLayout(m_ipInfoLayout);
    ipInfo->addStretch();

    layout->addLayout(ipInfo);
    layout->addSpacing(20);
    layout->addWidget(m_serverLineEdit, 0, Qt::AlignHCenter);
    layout->addWidget(m_regionBox, 0, Qt::AlignHCenter);
    layout->addStretch();

    m_serverLineEdit->setText(GetSettingsString(kEndPointControlServerUrl));
    m_serverLineEdit->setReadOnly(GetSettingsBool(kEndPointControlLockServer));

    QTimer::singleShot(0, m_serverLineEdit, &LineEdit::editingFinished);

    nextButton->setEnabled(false);

    q_ptr->setLayout(layout);

    networkAccessManager = new QNetworkAccessManager(this);
}

void ControlPlatformFramePrivate::initConnection()
{
    connect(networkAccessManager, &QNetworkAccessManager::finished, this,
            &ControlPlatformFramePrivate::onNetworkFinished);

    connect(m_serverLineEdit, &LineEdit::editingFinished, this, [=] {
        const QString& value = m_serverLineEdit->text();
        if (value.isEmpty()) return;

        m_serverUrl = QUrl(QString("%1/api/all_area").arg(value));
        networkAccessManager->get(QNetworkRequest(m_serverUrl));
    });

    connect(m_regionBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ControlPlatformFramePrivate::onRegionSelected);

}

ControlPlatformFrame::ControlPlatformFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new ControlPlatformFramePrivate(this))

{
    this->setStyleSheet("QLabel{color: white;}");

    m_private->onNetworkStateChanged();
}

ControlPlatformFrame::~ControlPlatformFrame()
{

}

void ControlPlatformFrame::init()
{

}

void ControlPlatformFrame::finished()
{
    // save config
    RegionInfo info = m_private->m_regionModel->findInfoByIndex(m_private->m_regionBox->currentIndex());

    QFile file("/etc/dmcg/config.json");
    if (file.open(QIODevice::Text | QIODevice::WriteOnly)) {
        QJsonObject obj;
        obj["protocol"]    = m_private->m_serverUrl.scheme();
        obj["server_port"] = m_private->m_serverUrl.port();
        obj["server_host"] = m_private->m_serverUrl.host();
        obj["area_id"]     = info.Id;
        obj["Area"] = info.toJson();
        QJsonDocument doc;
        doc.setObject(obj);
        file.write(doc.toJson());
        file.close();
    }

    emit requestFinished();
}

bool ControlPlatformFrame::shouldDisplay() const
{
    return !(GetSettingsBool(kSystemInfoSetupAfterReboot) || GetSettingsBool(kSkipControlPlatformPage));
}

QString ControlPlatformFrame::returnFrameName() const
{
    return ::QObject::tr("Control Platform");
}

bool ControlPlatformFrame::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        m_private->nextButton->setText(::QObject::tr("Next"));
        m_private->m_serverLineEdit->setPlaceholderText(::QObject::tr("Server Address"));
        m_private->m_titleLbl->setText(::QObject::tr("Set Control Region"));
        m_private->m_subTitleLbl->setText(::QObject::tr("Set the region for UOS EndPoint Management Platform"));
    }

    return QWidget::event(event);
}

void ControlPlatformFramePrivate::onNetworkFinished(QNetworkReply* reply)
{
    QTextCodec*   codec = QTextCodec::codecForName("utf8");
    const QString all   = codec->toUnicode(reply->readAll());
    reply->deleteLater();

    QJsonArray array = QJsonDocument::fromJson(all.toUtf8()).array();

    QList<RegionInfo> list;
    for (const QJsonValue& value : array) {
        RegionInfo info;
        info.formJson(value.toObject());
        list << info;
    }

    m_regionModel->setList(list);
}

void ControlPlatformFramePrivate::onRegionSelected()
{
    nextButton->setEnabled(true);
}

void ControlPlatformFramePrivate::onNetworkStateChanged() {
    ClearLayout(m_macInfoLayout);
    ClearLayout(m_ipInfoLayout);

    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface& interface : interfaces) {
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        for (const QNetworkAddressEntry& entry : entryList) {
            const QHostAddress& address = entry.ip();
            if (address == QHostAddress::LocalHost || address == QHostAddress::LocalHostIPv6 || address == QHostAddress::AnyIPv6) {
                continue;
            }
            bool ok = false;
            address.toIPv4Address(&ok);
            if (!ok) {
                continue;
            }
            m_macInfoLayout->addWidget(new QLabel(QString("mac: %1").arg(interface.hardwareAddress())));
            m_ipInfoLayout->addWidget(new QLabel(QString("IP: %1").arg(address.toString())));
        }
    }
}
}// namespace installer
#include "control_platform_frame.moc"
