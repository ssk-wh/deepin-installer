#ifndef INSTALL_LOG_FRAME_H
#define INSTALL_LOG_FRAME_H

#include <QFrame>

namespace {
    const int kInstallLogWidth = 530;
    const int kInstallLogHeight = 335;
}

class QPlainTextEdit;
class QTimer;

namespace installer {

class InstallLogFrame : public QFrame
{
    Q_OBJECT
public:
    explicit InstallLogFrame(QFrame *parent = nullptr);
    void setLogPath(const QString &);
    void updateSize(const QSize &size);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QPlainTextEdit *m_installLog = nullptr;
    QTimer* m_timer = nullptr;
    QString m_logPath;
};

}

#endif // INSTALL_LOG_FRAME_H
