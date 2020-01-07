#ifndef INSTALLER_UI_FRAMES_USER_AGREEMENT_FRAME_H
#define INSTALLER_UI_FRAMES_USER_AGREEMENT_FRAME_H

#include <QFrame>
#include <QVBoxLayout>
#include <QLocale>

class QLabel;
class QPushButton;
class QScrollArea;
class QButtonGroup;
class QAbstractButton;

namespace installer {
namespace {
    const int kChineseToggleButtonId = 0;
    const int kEnglishToggleButtonId = 1;
}

class NavButton;
class PointerButton;
class UserAgreementFrame : public QFrame
{
    Q_OBJECT
public:
    explicit UserAgreementFrame(QWidget *parent = nullptr);  
    void setUserAgreement(const QString &primaryFileName, const QString &secondaryFileName = QString(""));
    void setCheckedButton(int buttonId);

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
    PointerButton* m_chineseButton = nullptr;
    PointerButton* m_englishButton = nullptr;
    QAbstractButton* m_currentButton = nullptr;
    QButtonGroup* m_buttonGroup = nullptr;
    QLabel *m_sourceLbl;
    NavButton *m_back;
    QScrollArea *m_sourceScrollArea;
    QLocale::Language m_language;
    int m_nextFileIndex;
    QStringList m_fileNames;
};
}
#endif // INSTALLER_UI_FRAMES_USER_AGREEMENT_FRAME_H
