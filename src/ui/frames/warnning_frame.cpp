#include "warnning_frame.h"

#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/utils/widget_util.h"
#include "service/settings_name.h"
#include "service/settings_manager.h"

namespace {
    const int kWarnningDialogWidth = 400;
    const int kWarnningDialogHeight = 220;
}

namespace installer {


WarnningFrame::WarnningFrame(FrameProxyInterface *frameProxyInterface, QWidget *parent)
    : ChildFrameInterface(frameProxyInterface, parent)
{
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
    m_spliLabel->setVisible(isuse);
}

void WarnningFrame::useCancelButton(bool isuse)
{
    m_spliLabel->setVisible(isuse);
    m_cancelButton->setVisible(isuse);
}

bool WarnningFrame::focusSwitch()
{
    if (m_current_focus_widget == nullptr) {
        this->setCurentFocus(m_cancelButton);
    } else if (m_cancelButton == m_current_focus_widget) {
        this->setCurentFocus(m_enterButton);
    } else if (m_enterButton == m_current_focus_widget) {
        this->setCurentFocus(m_cancelButton);
    }

    return true;
}

bool WarnningFrame::doSpace()
{
    return true;
}

bool WarnningFrame::doSelect()
{
    if (m_cancelButton == m_current_focus_widget) {
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
    this->setCurentFocus(m_cancelButton);
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
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void WarnningFrame::initConnections()
{
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

    QFont font(QApplication::font());
    // 设置的字体大小是跟随像素的
    font.setPixelSize(installer::GetSettingsInt(installer::kSystemDefaultFontSize) + 4);
    font.setFamily(installer::GetUIDefaultFont());

    m_titleLabel = new QLabel(this);
    m_titleLabel->setFocusPolicy(Qt::NoFocus);
    m_titleLabel->setFont(font);
    m_titleLabel->setStyleSheet("QLabel{color:rgba(0, 0, 0, 0.9); line-height:30px;}");

    m_commentLabel = new CommentLabel(this);
    m_commentLabel->setFocusPolicy(Qt::NoFocus);
    m_commentLabel->setStyleSheet("QLabel{color:rgba(0, 0, 0, 0.6); line-height:30px;}");
    m_commentLabel->setFixedWidth(380);
    m_commentLabel->setWordWrap(true);
    m_commentLabel->setAlignment(Qt::AlignCenter);

    m_enterButtonText = "OK";
    m_cancelButtonText = "Cancel";

    m_enterButton = new QPushButton(this);
    m_enterButton->setText(::QObject::tr("OK"));
    m_enterButton->setFixedSize(170, 36);
    m_enterButton->setStyleSheet("QPushButton{color:#414D68; line-height:30px;}");
    //m_affirmButton->setFocusPolicy(Qt::TabFocus);

    m_spliLabel = new QLabel(this);
    m_spliLabel->setFocusPolicy(Qt::NoFocus);
    m_spliLabel->setStyleSheet("QLabel{background-color:rgba(0, 0, 0, 0.1);}");
    m_spliLabel->setMaximumSize(2, 28);
    m_spliLabel->setMinimumSize(2, 28);

    m_cancelButton = new QPushButton(this);
    m_cancelButton->setText(::QObject::tr("Cancel"));
    m_cancelButton->setFixedSize(170, 36);
    m_cancelButton->setStyleSheet("QPushButton{color:#414D68; line-height:30px;}");
    //m_affirmButton->setFocusPolicy(Qt::TabFocus);

    m_buttonGroupLayout = new QHBoxLayout(this);
    m_buttonGroupLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonGroupLayout->setSpacing(6);
    m_buttonGroupLayout->addWidget(m_cancelButton);
    m_buttonGroupLayout->addWidget(m_spliLabel);
    m_buttonGroupLayout->addWidget(m_enterButton);

    QWidget *buttonGroupWidget = new QWidget(this);
    buttonGroupWidget->setFocusPolicy(Qt::NoFocus);
    buttonGroupWidget->setLayout(m_buttonGroupLayout);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(10, 10, 10, 10);
    vLayout->setSpacing(6);
    vLayout->addWidget(m_icoButton, 0, Qt::AlignLeft | Qt::AlignTop);
    vLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    vLayout->addWidget(m_commentLabel, 0, Qt::AlignCenter);
    vLayout->addWidget(buttonGroupWidget, 0, Qt::AlignHCenter | Qt::AlignBottom);

    setLayout(vLayout);
    setFixedSize(kWarnningDialogWidth, kWarnningDialogHeight);
}

}
