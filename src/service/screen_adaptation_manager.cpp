#include "screen_adaptation_manager.h"

#include <DApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QDebug>

DWIDGET_USE_NAMESPACE

installer::ScreenAdaptationManager *installer::ScreenAdaptationManager::instance()
{
    static ScreenAdaptationManager sam;
    return &sam;
}

QPixmap installer::ScreenAdaptationManager::adapterPixmap(const QString &file)
{
    QPixmap pixmap(file);
    return adapterPixmap(pixmap);
}

QPixmap installer::ScreenAdaptationManager::adapterPixmap(const QPixmap &pixmap)
{
    return pixmap.scaled(pixmap.size().width() / m_zoomRatio, pixmap.size().height() / m_zoomRatio,
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}


void installer::ScreenAdaptationManager::init()
{
    QRect rect = qApp->desktop()->screenGeometry();
#ifdef QT_DEBUG
    qDebug() << "rect.width() = " << rect.width();
    qDebug() << "rect.height() = " << rect.height();

    qDebug() << "rect() = " << rect;
#endif // QT_DEBUG

    if (rect.width() == 800 && rect.height() == 600) {
        m_zoomRatio = 2;
    }

    qDebug() << "m_zoomRatio = " << m_zoomRatio;
}

installer::ScreenAdaptationManager::ScreenAdaptationManager()
{
    init();
}

