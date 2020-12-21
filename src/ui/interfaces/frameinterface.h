/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <justforlxz@outlook.com>
 *
 * Maintainer: justforlxz <justforlxz@outlook.com>
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

#ifndef FRAMEINTERFACE_H
#define FRAMEINTERFACE_H

#include <QApplication>
#include <QKeyEvent>

#include "frameproxyinterface.h"

#include <QApplication>

namespace installer {
enum class FrameType {
    Frame,              // 主页面
    PopupExtFrame,      // 以弹窗全屏显示的主页面
    NoLeftLabelExtFrame,// 在左侧列表中无标签的主页面
    FullScreenExtFrame, // 全屏显示的主页面
    ChildFrame,         // 子页面
};

class BaseFrameInterface : public QWidget {
    Q_OBJECT

public:
    explicit BaseFrameInterface(FrameType frameType, FrameProxyInterface* inter, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_proxy(inter)
        , m_frameType(frameType)
    {
    }

    inline FrameType frameType() {
        return m_frameType;
    }

    inline void setFrameType(FrameType type) {
        m_frameType = type;
    }

    virtual QString returnFrameName() const {
        return "";
    }

protected:
    virtual bool focusSwitch() {
        return false;
    }

    virtual bool doSpace() {
        return false;
    }

    virtual bool doSelect() {
        return false;
    }

    virtual bool directionKey(int keyvalue) {
        return false;
    }

    void setCurentFocus(QWidget *wid) {
        if (wid != nullptr) {
            m_current_focus_widget = wid;
            //rgb(0, 160, 230) border-color: rgb(255, 160, 230); border:5px solid red;
            //m_current_focus_widget->setStyleSheet("QWidget:focus{padding: -1;}");
            m_current_focus_widget->setFocus();
        }
    }

    bool isFocus(QWidget *wid) {
        return wid == m_current_focus_widget;
    }

    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *key = dynamic_cast<QKeyEvent*>(event);
            if (key != nullptr) {
                int key_value = key->key();
                switch (key_value) {
                    case Qt::Key_Tab:    return focusSwitch();
                    case Qt::Key_Space:  return doSpace();
                    case Qt::Key_Return: return doSelect();
                    case Qt::Key_Up:     return directionKey(Qt::Key_Up);
                    case Qt::Key_Down:   return directionKey(Qt::Key_Down);
                    case Qt::Key_Left:   return directionKey(Qt::Key_Left);
                    case Qt::Key_Right:  return directionKey(Qt::Key_Right);
                    default: return QWidget::eventFilter(watched, event);
                }
            } else {
                return QWidget::eventFilter(watched, event);
            }
        } else {
            return QWidget::eventFilter(watched, event);
        }
    }

    void showEvent(QShowEvent *evnet) override {
        qApp->installEventFilter(this);
        return QWidget::showEvent(evnet);
    }

    void hideEvent(QHideEvent *evnet) override {
        qApp->removeEventFilter(this);
        m_current_focus_widget = nullptr;
        return QWidget::hideEvent(evnet);
    }

protected:
    FrameProxyInterface* m_proxy = nullptr;
    QWidget *m_current_focus_widget = nullptr;

private:
    FrameType m_frameType;
};

class FrameInterface : public BaseFrameInterface {
    Q_OBJECT
    friend class FrameInterfacePrivate;

public:
    explicit FrameInterface(FrameProxyInterface* inter, QWidget* parent = nullptr)
        : BaseFrameInterface(FrameType::Frame, inter, parent)
    {
        //setFocusPolicy(Qt::TabFocus);
    }

    virtual ~FrameInterface() {}

    // 读取配置文件
    virtual void init() = 0;

    // 写入配置文件
    virtual void finished() = 0;

    // 读取配置文件确认当前页面是否可用
    virtual bool shouldDisplay() const = 0;

    // 控制是否允许上一步
    virtual bool allowPrevious() const {
        return true;
    }

signals:
    void updateNextButton();

protected:
    void showEvent(QShowEvent *event) override {
        emit updateNextButton();
        qApp->setActiveWindow(this);
        return BaseFrameInterface::showEvent(event);
    }

private:
    void nextFrame() {
        m_proxy->nextFrame();
    }
};

class ChildFrameInterface : public BaseFrameInterface
{
    Q_OBJECT

public:
    explicit ChildFrameInterface(FrameProxyInterface* inter, QWidget* parent = nullptr)
        : BaseFrameInterface(FrameType::ChildFrame, inter, parent)
    {
    }

    virtual ~ChildFrameInterface() {}

};
}  // namespace installer

#endif  // FRAMEINTERFACE_H
