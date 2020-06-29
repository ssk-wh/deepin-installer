#include "di_scrollarea.h"
#include "base/file_util.h"

#include <QScroller>
#include <QScrollBar>
#include <QEvent>
#include <QTimer>
#include <QResizeEvent>

namespace installer {

namespace {
    const int kScrollAreaWidth = 468;
}

DIScrollArea::DIScrollArea(QWidget *parent)
    : QScrollArea (parent)
{
    initUI();
}

void DIScrollArea::setWidget(QWidget *widget)
{
    if (this->widget()) {
        this->widget()->removeEventFilter(this);
    }

    QScrollArea::setWidget(widget);
    widget->installEventFilter(this);
}

void DIScrollArea::initUI()
{
    setWidgetResizable(true);
    setFocusPolicy(Qt::TabFocus);
    setFrameStyle(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    setContentsMargins(0, 0, 0, 0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::NoContextMenu);
    setStyleSheet("background: transparent;");
    QScroller::grabGesture(this, QScroller::TouchGesture);
    setMaximumWidth(kScrollAreaWidth);

    m_scrollBar = verticalScrollBar();
    m_scrollBar->setParent(this);
    m_scrollBar->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollBar->move(kScrollAreaWidth - 8, 0);
    m_scrollBar->raise();
    m_scrollBar->setStyleSheet(ReadFile(":/styles/di_scrollarea.css"));
}

bool DIScrollArea::event(QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        QResizeEvent* e = static_cast<QResizeEvent*>(event);
        m_scrollBar->move(e->size().width() - 8 - 3, 0);
        m_scrollBar->setFixedSize(QSize(8, e->size().height() - 5));
    }

    return QScrollArea::event(event);
}

bool DIScrollArea::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == widget() && event->type() == QEvent::Resize) {
        QTimer::singleShot(0, this, [=] {
            m_scrollBar->setVisible(widget()->height() > height());
            setFixedWidth(widget()->width());
        });
    }

    return QScrollArea::eventFilter(watched, event);
}

}
