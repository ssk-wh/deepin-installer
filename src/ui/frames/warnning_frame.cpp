#include "warnning_frame.h"

#include <QPainterPath>

#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/utils/widget_util.h"
#include "service/settings_name.h"
#include "service/settings_manager.h"

namespace {
    const int kWarnningDialogWidth = 420;
    const int kWarnningDialogHeight = 179;
    const int kWarnningDialogSmallWidth = 380;
    const int kWarnningDialogSmallHeight = 159;
}

namespace installer {


WarnningFrame::WarnningFrame(FrameProxyInterface *frameProxyInterface, QWidget *parent)
    : ChildFrameInterface(frameProxyInterface, parent)
{
    setObjectName("WarnningFrame");
    initUI();
    initConnections();
}

void WarnningFrame::setTitle(const QString &titletext)
{
    m_titleText = titletext;
}

void WarnningFrame::setComment(const QString &commenttext)
{
    m_commentText = commenttext;
}

void WarnningFrame::setEnterButtonText(const QString &buttontext)
{
    m_enterButtonText = buttontext;
}

void WarnningFrame::setCancelButtonText(const QString &buttontext)
{
    m_cancelButtonText = buttontext;
}

void WarnningFrame::useTitle(bool isuse)
{
    m_titleLabel->setVisible(isuse);
}

void WarnningFrame::useComment(bool isuse)
{
    m_commentLabel->setVisible(isuse);
}

void WarnningFrame::useEnterButton(bool isuse)
{
    m_enterButton->setVisible(isuse);

    if (m_enterButton->isVisible() && m_cancelButton->isVisible()) {
        setMaximumWidth(kWarnningDialogWidth);
        setMinimumWidth(kWarnningDialogWidth);
        m_commentLabel->setFixedWidth(kWarnningDialogWidth - 40);
        m_enterButton->setFixedSize(179, 36);
        m_cancelButton->setFixedSize(179, 36);
    } else {
        setMaximumWidth(kWarnningDialogSmallWidth);
        setMinimumWidth(kWarnningDialogSmallWidth);
        m_commentLabel->setFixedWidth(kWarnningDialogSmallWidth - 40);
        m_enterButton->setFixedSize(159, 36);
        m_cancelButton->setFixedSize(159, 36);
    }
}

void WarnningFrame::useCancelButton(bool isuse)
{
    m_cancelButton->setVisible(isuse);

    if (m_enterButton->isVisible() && m_cancelButton->isVisible()) {
        setMaximumWidth(kWarnningDialogWidth);
        setMinimumWidth(kWarnningDialogWidth);
        m_commentLabel->setFixedWidth(kWarnningDialogWidth - 40);
        m_enterButton->setFixedSize(179, 36);
        m_cancelButton->setFixedSize(179, 36);
    } else {
        setMaximumWidth(kWarnningDialogSmallWidth);
        setMinimumWidth(kWarnningDialogSmallWidth);
        m_commentLabel->setFixedWidth(kWarnningDialogSmallWidth - 40);
        m_enterButton->setFixedSize(360, 36);
        m_cancelButton->setFixedSize(360, 36);
    }
}

void WarnningFrame::setEnterButtonStyle(const QString &buttonstyle)
{
    m_enterButton->setStyleSheet(buttonstyle);
}

void WarnningFrame::setCancelButtonStyle(const QString &buttonstyle)
{
    m_cancelButton->setStyleSheet(buttonstyle);
}

bool WarnningFrame::focusSwitch()
{
    if (m_current_focus_widget == nullptr) {
        if (m_cancelButton->isVisible()) {
            this->setCurentFocus(m_cancelButton);
        } else if (m_enterButton->isVisible()) {
            this->setCurentFocus(m_enterButton);
        } else {
            this->setCurentFocus(m_closeButton);
        }
    } else if (m_cancelButton == m_current_focus_widget) {
        if (m_enterButton->isVisible()) {
            this->setCurentFocus(m_enterButton);
        } else {
            this->setCurentFocus(m_closeButton);
        }
    } else if (m_enterButton == m_current_focus_widget) {
        this->setCurentFocus(m_closeButton);
    } else if (m_closeButton == m_current_focus_widget) {
        if (m_cancelButton->isVisible()) {
            this->setCurentFocus(m_cancelButton);
        } else if (m_enterButton->isVisible()) {
            this->setCurentFocus(m_enterButton);
        }
    }

    return true;
}

bool WarnningFrame::doSpace()
{
    return true;
}

bool WarnningFrame::doSelect()
{
    if (m_closeButton == m_current_focus_widget) {
        emit m_closeButton->clicked();
    } else if (m_cancelButton == m_current_focus_widget) {
        emit m_cancelButton->clicked();
    } else if (m_enterButton == m_current_focus_widget) {
        emit m_enterButton->clicked();
    }
    return true;
}

bool WarnningFrame::directionKey(int keyvalue)
{
    if (m_current_focus_widget == nullptr) {
        this->setCurentFocus(m_cancelButton);
    }

    switch (keyvalue) {
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    case Qt::Key_Left: {
            this->setCurentFocus(m_cancelButton);
        }
        break;
    case Qt::Key_Right: {
            this->setCurentFocus(m_enterButton);
        }
        break;
    }
    return true;
}

void WarnningFrame::showEvent(QShowEvent *event)
{
    m_titleLabel->setText(::QObject::tr(m_titleText.toLatin1()));
    m_commentLabel->setText(::QObject::tr(m_commentText.toLatin1()));
    m_enterButton->setText(::QObject::tr(m_enterButtonText.toLatin1()));
    m_cancelButton->setText(::QObject::tr(m_cancelButtonText.toLatin1()));
    this->clearFocus();
    return ChildFrameInterface::showEvent(event);
}

void WarnningFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        m_titleLabel->setText(::QObject::tr(m_titleText.toLatin1()));
        m_commentLabel->setText(::QObject::tr(m_commentText.toLatin1()));
        m_enterButton->setText(::QObject::tr(m_enterButtonText.toLatin1()));
        m_cancelButton->setText(::QObject::tr(m_cancelButtonText.toLatin1()));
    } else {
      ChildFrameInterface::changeEvent(event);
    }
}

void WarnningFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), QColor(255, 255, 255));

    return QWidget::paintEvent(event);
}

void WarnningFrame::initConnections()
{
    connect(m_closeButton, &QPushButton::clicked, this, &WarnningFrame::quitCanceled);
    connect(m_cancelButton, &QPushButton::clicked, this, &WarnningFrame::quitCanceled);
    connect(m_enterButton, &QPushButton::clicked, this, &WarnningFrame::quitEntered);
}

void WarnningFrame::initUI()
{
    const auto ratio = devicePixelRatioF();
    m_icoButton = new QPushButton(this);
    m_icoButton->setFocusPolicy(Qt::NoFocus);
    m_icoButton->setStyleSheet("QPushButton{border-image:url(:/images/interaction_warning.svg);}");
    m_icoButton->setFixedSize(32 * ratio, 32 * ratio);

    m_closeButton = new QPushButton(this);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setStyleSheet("QPushButton{border:none; border-radius:10px; background-color:rgba(0, 0, 0, 0);} \
                            QPushButton:hover{border:none; border-radius:10px; background-color:rgba(0, 0, 0, 0.05);}");
    m_closeButton->setFixedSize(32 * ratio, 32 * ratio);
    m_closeButton->setIconSize(QSize(50, 50));
    m_closeButton->setIcon(QIcon(":/images/close_normal.svg"));

    QHBoxLayout *titlebuttonGroupLayout = new QHBoxLayout(this);
    titlebuttonGroupLayout->setContentsMargins(0, 0, 0, 0);
    titlebuttonGroupLayout->addWidget(m_icoButton, 0, Qt::AlignLeft);
    titlebuttonGroupLayout->addWidget(m_closeButton, 0, Qt::AlignRight);

    QWidget *titlebuttonGroupWidget = new QWidget(this);
    titlebuttonGroupWidget->setFocusPolicy(Qt::NoFocus);
    titlebuttonGroupWidget->setLayout(titlebuttonGroupLayout);

    //QFont font(QApplication::font());
    //// 设置的字体大小是跟随像素的
    //font.setPixelSize(installer::GetSettingsInt(installer::kSystemDefaultFontSize) + 4);
    //font.setFamily(installer::GetUIDefaultFont());

    m_titleLabel = new QLabel(this);
    m_titleLabel->setFocusPolicy(Qt::NoFocus);
    //m_titleLabel->setFont(font);
    m_titleLabel->setStyleSheet("QLabel{color:rgba(0, 0, 0, 0.9);}");

    m_commentLabel = new CommentLabel(this);
    m_commentLabel->setFocusPolicy(Qt::NoFocus);
    m_commentLabel->setStyleSheet("QLabel{color:rgba(0, 0, 0, 0.6);}");
    m_commentLabel->setFixedWidth(kWarnningDialogWidth - 40);
    m_commentLabel->setWordWrap(true);
    m_commentLabel->setAlignment(Qt::AlignCenter);
    m_commentLabel->setContentsMargins(10,0,10,0);

    m_enterButtonText = "OK";
    m_cancelButtonText = "Cancel";

    m_enterButton = new QPushButton(this);
    m_enterButton->setFocusPolicy(Qt::NoFocus);
    m_enterButton->setText(::QObject::tr("OK"));
    m_enterButton->setFixedSize(179, 36);
    m_enterButton->setStyleSheet("QPushButton{ color:#414D68; border:1px solid; border-color:rgba(0, 0, 0, 0.03); border-radius:8px; background-color:rgb(230, 230, 230); } \
                            QPushButton:hover{ color:#414D68; border:1px solid; border-color:rgba(0, 0, 0, 0.03); border-radius:8px; background-color:rgb(205, 205, 205); } \
                          QPushButton:pressed{ color:#0081FF; border:1px solid; border-color:rgba(0, 0, 0, 0.03); border-radius:8px; background-color:rgb(188, 196, 208); }");

    m_cancelButton = new QPushButton(this);
    m_cancelButton->setFocusPolicy(Qt::NoFocus);
    m_cancelButton->setText(::QObject::tr("Cancel"));
    m_cancelButton->setFixedSize(179, 36);
    m_cancelButton->setStyleSheet("QPushButton{ color:#414D68; border:1px solid; border-color:rgba(0, 0, 0, 0.03); border-radius:8px; background-color:rgb(230, 230, 230); } \
                             QPushButton:hover{ color:#414D68; border:1px solid; border-color:rgba(0, 0, 0, 0.03); border-radius:8px; background-color:rgb(205, 205, 205); } \
                           QPushButton:pressed{ color:#0081FF; border:1px solid; border-color:rgba(0, 0, 0, 0.03); border-radius:8px; background-color:rgb(188, 196, 208); }");

    m_buttonGroupLayout = new QHBoxLayout(this);
    m_buttonGroupLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonGroupLayout->setSpacing(10);
    m_buttonGroupLayout->addWidget(m_cancelButton);
    m_buttonGroupLayout->addWidget(m_enterButton);

    QWidget *buttonGroupWidget = new QWidget(this);
    buttonGroupWidget->setFocusPolicy(Qt::NoFocus);
    buttonGroupWidget->setLayout(m_buttonGroupLayout);

    QSpacerItem *licenseVSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    licenseVSpacer->setAlignment(Qt::AlignHCenter);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(10, 10, 10, 10);
    vLayout->setSpacing(5);
    vLayout->addWidget(titlebuttonGroupWidget, 0, Qt::AlignTop);
    vLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    vLayout->addWidget(m_commentLabel, 0, Qt::AlignCenter);
    vLayout->addSpacerItem(licenseVSpacer);
    vLayout->addWidget(buttonGroupWidget, 0, Qt::AlignBottom);

    setLayout(vLayout);
    setMaximumWidth(kWarnningDialogWidth);
    setMinimumWidth(kWarnningDialogWidth);
}

}
