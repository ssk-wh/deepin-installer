#pragma once

#include <QWidget>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace installer {

class ComponentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentWidget(bool singleSelected, QWidget* parent = nullptr);

    void setTitle(const QString& title)
    {
        m_titleLabel->setText(title);
    }

    void setDesc(const QString& desc)
    {
        m_descLabel->setText(desc);
    }

    void setSelected(bool selected);
    bool isSelected() const;

protected:
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void clicked();

private:
    QRadioButton* m_radioBotton;
    QCheckBox* m_checkBox;
    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
};

}
