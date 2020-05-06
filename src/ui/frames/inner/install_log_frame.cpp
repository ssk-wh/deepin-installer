#include "install_log_frame.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QScrollBar>

installer::InstallLogFrame::InstallLogFrame(QFrame *parent) :
    QFrame(parent)
{   
    QFont font;
    font.setPointSize(11);
    QPalette palette;
    palette.setColor(QPalette::Text, QColor(66, 154, 216));
    m_installLog = new QPlainTextEdit(this);
    m_installLog->setFixedSize(kInstallLogWidth, kInstallLogHeight);
    m_installLog->setPalette(palette);
    m_installLog->setFont(font);
    m_installLog->setReadOnly(true);
    m_installLog->setStyleSheet("QFrame{background:rgba(0,0,0,0.1);}");

    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(m_installLog, 0, Qt::AlignHCenter);

    setLayout(main_layout);
}

void installer::InstallLogFrame::setLogPath(const QString &path)
{
#ifdef QT_DEBUG_zdd
    QTimer* m_test = new QTimer;
    m_test->setInterval(10);

    connect(m_test, &QTimer::timeout, this, [=] {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to open the log file. file: " << path;
        }

        static int cnt = 0;
        file.write(QString("open file: " + path + QString::number(cnt++) + "\n").toStdString().c_str());
    });

    m_test->start();
#endif // QT_DEBUG

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

        while (!file.atEnd()) {
            m_installLog->insertPlainText(file.readLine());
            m_installLog->moveCursor(QTextCursor::End);
        }
    });

    m_timer->start();
}

