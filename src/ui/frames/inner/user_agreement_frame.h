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
namespace {
    const int kChineseToggleButtonId = 0;
    const int kEnglishToggleButtonId = 1;
}

class UserAgreementFrame : public QFrame
{
    Q_OBJECT
public:
    explicit UserAgreementFrame(QWidget *parent = nullptr);
    void setUserAgreement(const QString &primaryFileName, const QString &secondaryFileName = QString(""));
    void setCheckedButton(int buttonId);
    void setTitle(const QString &text);

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
    void toggleLicense(QAbstractButton* button);
    void updateLicenseText();

private:
    QLabel *m_logoLbl;
    QLabel *m_subTitle;
    DButtonBoxButton* m_chineseButton = nullptr;
    DButtonBoxButton* m_englishButton = nullptr;
    QAbstractButton* m_currentButton = nullptr;
    DButtonBox* m_buttonBox = nullptr;
    QWidget* m_buttonBoxWidget = nullptr;
    QLabel *m_sourceLbl;
    QPushButton *m_back;
    QScrollArea *m_sourceScrollArea;
    QLocale::Language m_language;
    int m_nextFileIndex;
    QStringList m_fileNames;
    QList<DButtonBoxButton *> m_btnlist;
};
}
#endif // INSTALLER_UI_FRAMES_USER_AGREEMENT_FRAME_H
