#ifndef VERIFY_DIALOG_H
#define VERIFY_DIALOG_H

#include <QWidget>
#include <DDialog>
#include <QProgressBar>

DWIDGET_USE_NAMESPACE

class VerifyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit VerifyDialog(QWidget *parten = nullptr);

    void setValue(int value);

private:
    QProgressBar *m_progressWidget = nullptr;
};

class Verify : public QObject
{
    Q_OBJECT
public:
    void start();

signals:
    void done(bool flag);
};

#endif // VERRIFY_DIALOG_H
