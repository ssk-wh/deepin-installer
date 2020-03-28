#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace installer {

class PartitionTableWarningWidget : public QPushButton
{
    Q_OBJECT

public:
    explicit PartitionTableWarningWidget(QWidget* parent = nullptr);

    bool isSelected() const;

    void setTitle(const QString& title);
    void setDesc(const QString& desc);
    void setSelected(bool selected);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
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
