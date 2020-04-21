#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <DButtonBox>

DWIDGET_USE_NAMESPACE

namespace installer {

class PartitionTableWarningWidget : public DButtonBoxButton
{
    Q_OBJECT

public:
    explicit PartitionTableWarningWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setDesc(const QString& desc);

    void updateCheckedAppearance();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
    QLabel *m_checkedLabel;
    bool m_isflag;
};

}
