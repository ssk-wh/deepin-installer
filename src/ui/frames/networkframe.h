#ifndef NETWORKFRAME_H
#define NETWORKFRAME_H

#include <QWidget>
#include <memory>

class QRegularExpressionValidator;
class QLabel;
namespace installer {
class NavButton;
class NetworkEditWidget;
class NetworkFrame : public QWidget {
    Q_OBJECT
public:
    explicit NetworkFrame(QWidget* parent = nullptr);

signals:
    void requestNext();

protected:
    bool event(QEvent *event) override;

private:
    void saveConf();

private:
    QLabel*                                      m_subTitle;
    NavButton*                                   m_skipButton;
    NavButton*                                   m_saveButton;
    NetworkEditWidget*                           m_currentNetworkEditWidget;
};
}  // namespace installer

#endif  // NETWORKFRAME_H
