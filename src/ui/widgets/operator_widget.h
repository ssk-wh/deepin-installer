/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPERATOR_WIDGET_H
#define OPERATOR_WIDGET_H

#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class QLabel;
class QCheckBox;

namespace installer {

class OperatorWidget : public DButtonBoxButton
{
    Q_OBJECT
public:
    explicit OperatorWidget(QWidget *parent = nullptr);

public:
    void setTitle(const QString &text);
    void setBody(const QString &text);
    void setSelectIcon(const QString &icon);
    void setSelectMode(bool isCancel);
    void setSelect(bool checked);

protected:
    virtual void selectChange();

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initUi();
    void initConnection();

private:
    QLabel* m_titleLabel;
    QLabel* m_bodyLabel;
    QLabel* m_selectIconLabel;
    QWidget* m_edgeWidget;
    bool    m_isCancelSelect;
};

}

#endif // OPERATOR_WIDGET_H
