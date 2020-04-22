#include "network_device_widget.h"
#include "ui/utils/widget_util.h"
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

namespace installer {

namespace {
    const int kNetworkDeviceWidgetWidth = 240;
    const int kNetworkDeviceWidgetHeight = 64;
    const int KQLabelWidth = 120;
    const int kTitleFont = 14; // 14pt
    const int kDescFont = 12; // 12pt
}

NetworkDeviceWidget::NetworkDeviceWidget(QWidget *parent)
    : DButtonBoxButton("", parent)
    , m_isflag(false)
{
    m_deviceName = new QLabel;
    m_deviceName->setObjectName("titleLabel");
    m_deviceName->setMinimumWidth(KQLabelWidth);

    QFont titleFont;
    titleFont.setPointSize(kTitleFont);
    m_deviceName->setFont(titleFont);

    m_descLabel = new QLabel;
    m_descLabel->setObjectName("descLabel");
    m_descLabel->setMinimumWidth(KQLabelWidth);
    m_descLabel->setWordWrap(true);
    m_descLabel->adjustSize();

    QFont descFont;
    descFont.setPointSize(kDescFont);
    m_descLabel->setFont(descFont);

    m_vLayout = new QVBoxLayout;
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setSpacing(0);
    m_vLayout->addWidget(m_deviceName, 0, Qt::AlignLeft);
    m_vLayout->addWidget(m_descLabel, 0, Qt::AlignLeft);

    m_checkedLabel = new QLabel;
    const QPixmap pixmap = installer::renderPixmap(":/images/select.svg");
    Q_ASSERT(!pixmap.isNull());
    m_checkedLabel->setPixmap(pixmap);
    m_checkedLabel->setFixedSize(pixmap.size() / devicePixelRatioF());
    m_checkedLabel->setVisible(isChecked());

    m_hLayout = new QHBoxLayout;
    m_hLayout->setContentsMargins(10, 10, 10, 10);
    m_hLayout->setSpacing(0);

    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addStretch();
    m_hLayout->addWidget(m_checkedLabel, 0, Qt::AlignRight);

    setFixedSize(kNetworkDeviceWidgetWidth, kNetworkDeviceWidgetHeight);
    setObjectName("PartitionTableWarningWidget");
    setContentsMargins(0, 0, 0, 0);
    setLayout(m_hLayout);
}

void NetworkDeviceWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath PaintPath;
    QColor color(0, 0, 0, 15);

    if (m_isflag) {
        color = QColor(0, 0, 0, 55);
    }

    PaintPath.addRoundedRect(rect(), 10, 10);
    painter.fillPath(PaintPath, color);

    QWidget::paintEvent(event);
}

void NetworkDeviceWidget::enterEvent(QEvent* event)
{
    m_isflag = true;

    update();

    QWidget::leaveEvent(event);
}

void NetworkDeviceWidget::leaveEvent(QEvent* event)
{
    m_isflag = false;

    update();

    QWidget::leaveEvent(event);
}

void NetworkDeviceWidget::setTitle(const QString &title)
{
    m_deviceName->setText(title);
}

void NetworkDeviceWidget::setDesc(const QString &desc)
{
    m_descLabel->setText(desc);
}

void NetworkDeviceWidget::updateCheckedAppearance()
{
    m_checkedLabel->setVisible(isChecked());
}

void NetworkDeviceWidget::setDeviceInfo(Device::Ptr device) {
    // TODO: replace Ethernet use actual device type.
    m_deviceName->setText(tr("Ethernet (%1)").arg(device->interfaceName()));
    m_device = device;
    m_networkOperate = new NetworkOperate(device);
}

NetworkManager::Device::Ptr NetworkDeviceWidget::getDevice() const {
    return m_device;
}

NetworkOperate *NetworkDeviceWidget::networkOperate() const
{
    return m_networkOperate;
}

}
