#include "component_widget.h"

namespace installer {

ComponentWidget::ComponentWidget(bool singleSelected, QWidget *parent)
    : QWidget (parent)
    , m_radioBotton(nullptr)
    , m_checkBox(nullptr)
{
    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("titleLabel");
    m_descLabel = new QLabel;
    m_descLabel->setObjectName("descLabel");
    m_hLayout = new QHBoxLayout;
    m_vLayout = new QVBoxLayout;
    m_vLayout->addWidget(m_titleLabel, 0, Qt::AlignLeft);
    m_vLayout->addSpacing(2);
    m_vLayout->addWidget(m_descLabel, 0, Qt::AlignLeft);
    if(singleSelected){
        m_radioBotton = new QRadioButton;
        m_radioBotton->setObjectName("radioBotton");
        m_radioBotton->setCheckable(true);
        m_radioBotton->setChecked(false);
        m_hLayout->addWidget(m_radioBotton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    }
    else {
        m_checkBox = new QCheckBox;
        m_checkBox->setObjectName("checkBox");
        m_checkBox->setCheckable(true);
        m_checkBox->setChecked(false);
        m_hLayout->addWidget(m_checkBox, 0, Qt::AlignLeft | Qt::AlignVCenter);
    }
    m_hLayout->addSpacing(5);
    m_hLayout->addLayout(m_vLayout);

    setLayout(m_hLayout);
}

void ComponentWidget::setSelected(bool selected)
{
    if (m_radioBotton) {
        m_radioBotton->setChecked(selected);
    }
    else {
        m_checkBox->setChecked(selected);
    }
}

bool ComponentWidget::isSelected() const
{
    return m_radioBotton ? m_radioBotton->isChecked() : m_checkBox->isChecked();
}

void ComponentWidget::mousePressEvent(QMouseEvent *event)
{
    setSelected(true);

    QWidget::mousePressEvent(event);

    emit clicked();
}

}
