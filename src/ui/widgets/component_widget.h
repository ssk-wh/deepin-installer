#pragma once

#include <QWidget>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace installer {

class ComponentWidget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool isHead READ isHead DESIGNABLE true SCRIPTABLE true)
    Q_PROPERTY(bool isTail READ isTail DESIGNABLE true SCRIPTABLE true)

public:
    explicit ComponentWidget(bool singleSelected, QWidget* parent = nullptr);

    inline bool isHead() const { return m_isHead; }
    inline bool isTail() const { return m_isTail; }
    bool isSelected() const;

    void setIsHead(bool head);
    void setIsTail(bool tail);
    void setTitle(const QString& title);
    void setDesc(const QString& desc);
    void setSelected(bool selected);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QRadioButton* m_radioBotton;
    QCheckBox* m_checkBox;
    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
    bool m_isHead;
    bool m_isTail;
    bool m_isflag;
};

}
