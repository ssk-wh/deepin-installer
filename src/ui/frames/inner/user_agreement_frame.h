#ifndef INSTALLER_UI_FRAMES_USER_AGREEMENT_FRAME_H
#define INSTALLER_UI_FRAMES_USER_AGREEMENT_FRAME_H

#include <QFrame>
#include <QVBoxLayout>
#include <QLocale>
#include <QList>

#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class QLabel;
class QPushButton;
class QScrollArea;
class QAbstractButton;

namespace installer {

class UserAgreementFrame : public QFrame
{
    Q_OBJECT
public:
    explicit UserAgreementFrame(QWidget *parent = nullptr);
    void setUserAgreement(const QString &primaryFileName, const QString &secondaryFileName = QString(""));

    bool focusSwitch();
    bool doSelect();
    bool directionKey(int keyvalue);

signals:
    // Emitted when cancel
    void back();

protected:
    // Update text of next_button_
    void changeEvent(QEvent* event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnect();
    void updateText();
    void updateLicenseText();

private:
    QLabel *m_logoLbl;
    QLabel *m_sourceLbl;
    QPushButton *m_back;
    QScrollArea *m_sourceScrollArea;
    QLocale::Language m_language;
    int m_nextFileIndex;
    QStringList m_fileNames;
};
}
#endif // INSTALLER_UI_FRAMES_USER_AGREEMENT_FRAME_H
