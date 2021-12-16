#include "link_button.h"

#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>

const int kIconWidth = 100;
const int kIconHeight = 10;

const int kTextWidth = 100;
const int kTextHeight = 31;

installer::LinkButton::LinkButton(QWidget *parent):
    QWidget(parent),
    m_toggle(false)
{
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(kIconWidth, kIconHeight);
    m_iconLabel->installEventFilter(this);

    QPalette textPalette;
    textPalette.setColor(QPalette::Text, QColor("#0082FA"));
    m_textLabel = new QLabel(this);
    m_textLabel->setPalette(textPalette);
    m_textLabel->setForegroundRole(QPalette::Text);
    m_textLabel->setAlignment(Qt::AlignCenter);
    m_textLabel->setFixedSize(kTextWidth, kTextHeight);
    m_textLabel->installEventFilter(this);

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(m_iconLabel);
    main_layout->addWidget(m_textLabel);

    setLayout(main_layout);

    connect(this, &LinkButton::toggle, this, &LinkButton::updateIcon);
}

void installer::LinkButton::setIconList(const QStringList &list)
{
    m_iconList = list;
    if (!list.isEmpty()) {
        setIcon(list.first());
    }
}

void installer::LinkButton::setIcon(const QString &icon)
{
    m_iconLabel->setPixmap(QPixmap(icon));
}

void installer::LinkButton::setText(const QString &text)
{
    m_textLabel->setText(text);
}

bool installer::LinkButton::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == m_textLabel || watched == m_iconLabel)
            && event->type() == QEvent::MouseButtonPress) {

        m_toggle = !m_toggle;
        Q_EMIT toggle(m_toggle);
    }

    return QWidget::eventFilter(watched, event);
}

void installer::LinkButton::updateIcon()
{
    m_currentIconPos++;
    if (m_currentIconPos >= m_iconList.size()) {
        m_currentIconPos = 0;
    }

    if (!m_iconList.isEmpty()) {
        setIcon(m_iconList.at(m_currentIconPos));
    }
}
