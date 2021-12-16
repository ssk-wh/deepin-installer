#include "install_log_frame.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QScrollBar>

namespace  {
    const int kFileKeepSize = 1024;
}

installer::InstallLogFrame::InstallLogFrame(QFrame *parent) :
    QFrame(parent)
{   
    QFont font;
    QPalette palette;
    palette.setColor(QPalette::Text, QColor(66, 154, 216));
    m_installLog = new QPlainTextEdit(this);
    m_installLog->setContextMenuPolicy(Qt::NoContextMenu);
    m_installLog->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_installLog->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_installLog->setFixedSize(kInstallLogWidth, kInstallLogHeight);
    m_installLog->setPalette(palette);
    m_installLog->setForegroundRole(QPalette::Text);
    m_installLog->setFont(font);
    m_installLog->setReadOnly(true);
    m_installLog->setStyleSheet("QPlainTextEdit{background-color: rgba(0, 0, 0, 0.03);"
                                "border-radius:8px;}");

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(m_installLog, 0, Qt::AlignHCenter);

    this->setLayout(main_layout);
}

void installer::InstallLogFrame::setLogPath(const QString &path)
{
    m_logPath = path;
    if (m_timer == nullptr) {
        m_timer = new QTimer;
        m_timer->setInterval(300);
    }

    connect(m_timer, &QTimer::timeout, this, [=] {
        QFile file(m_logPath);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open the log file. file: " << m_logPath;
        }

        qint64 pos = file.size() - kFileKeepSize < 0 ? 0 : file.size() - kFileKeepSize;
        file.seek(pos);

        m_installLog->clear();
        m_installLog->insertPlainText(file.readAll());
        m_installLog->moveCursor(QTextCursor::End);
        file.close();
    });
}

void installer::InstallLogFrame::showLogWithoutTimer(const QString &path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QString text = file.readAll();
        m_installLog->insertPlainText(text);
        m_installLog->setContextMenuPolicy(Qt::NoContextMenu);
        m_installLog->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
        m_installLog->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
        m_installLog->setReadOnly(true);
        m_installLog->moveCursor(QTextCursor::End);
        file.close();
    } else {
        qDebug() << "Failed to open the log file. file: " << path;
    }
}

void installer::InstallLogFrame::updateSize(const QSize &size)
{
    m_installLog->setFixedSize(size.width() - 50, size.height() - 50);
}

void installer::InstallLogFrame::showEvent(QShowEvent *event)
{
    if (m_timer != nullptr) {
        m_timer->start();
    }

    QFrame::showEvent(event);
}

void installer::InstallLogFrame::hideEvent(QHideEvent *event)
{
    if (m_timer != nullptr) {
        m_timer->stop();
    }

    QFrame::hideEvent(event);
}

