#include "style_delegate.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollBar>

QScrollArea *installer::StyleDelegate::area(QWidget *widget)
{
    QVBoxLayout *languageLayout = new QVBoxLayout;
    languageLayout->setContentsMargins(5, 5, 15, 0);
    languageLayout->addWidget(widget);

    QFrame *languageFrame = new QFrame;
    languageFrame->setContentsMargins(0, 0, 0, 0);
    languageFrame->setLayout(languageLayout);

    QScrollArea *languageArea = new QScrollArea;
    languageArea->setContentsMargins(0, 0, 0, 0);
    languageArea->setWidgetResizable(true);
    languageArea->setFocusPolicy(Qt::TabFocus);
    languageArea->setFrameStyle(QFrame::NoFrame);
    languageArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    languageArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    languageArea->setContextMenuPolicy(Qt::NoContextMenu);
    languageArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    languageArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    languageArea->setFixedWidth(470);
    languageArea->setWidget(languageFrame);

    return languageArea;
}
