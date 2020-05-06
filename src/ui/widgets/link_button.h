#ifndef LINK_BUTTON_H
#define LINK_BUTTON_H

#include <QFrame>

class QLabel;


namespace installer {

class LinkButton : public QWidget
{
    Q_OBJECT
public:
    explicit LinkButton(QWidget *parent = nullptr);
    void setIconList(const QStringList &list);
    void setIcon(const QString &icon);
    void setText(const QString &text);

signals:
    void toggle(bool toggle);

public slots:

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool updateIcon();

private:
    QLabel *m_iconLabel = nullptr;
    QLabel *m_textLabel = nullptr;

    QStringList m_iconList;
    int         m_currentIconPos = 0;
    bool        m_toggle;
};
}

#endif // LINK_BUTTON_H
