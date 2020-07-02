#include "screen_adaptation_manager.h"

#include <DApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QDebug>

DWIDGET_USE_NAMESPACE

namespace  {
    const int kXrandrWidth_832 = 832;
    const int kXrandrWidth_1024 = 1024;

    const int kXrandrHeight_624 = 624;
    const int kXrandrHeight_864 = 864;
    const int kXrandrHeight_480 = 480;
}

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
    return pixmap.scaled(pixmap.size().width() / m_widthZoomRatio,
                         pixmap.size().height() / m_heightZoomRatio,
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}


void installer::ScreenAdaptationManager::init()
{
    QRect rect = qApp->desktop()->screenGeometry();

    adapterWidth(rect.width());
    adapterHeight(rect.height());

    qDebug() << "rect.width() = " << rect.width();
    qDebug() << "rect.height() = " << rect.height();
    qDebug() << "rect() = " << rect;
    qDebug() << "m_widthZoomRatio = " << m_widthZoomRatio;
    qDebug() << "m_heightZoomRatio = " << m_heightZoomRatio;
}

void installer::ScreenAdaptationManager::adapterWidth(int width)
{
    if (width <= kXrandrWidth_832) {
        m_widthZoomRatio = 2;
    } else if (width <= kXrandrWidth_1024){
        m_widthZoomRatio = 1.5;
    }
}

void installer::ScreenAdaptationManager::adapterHeight(int height)
{
    if (height <= kXrandrHeight_480) {
        m_heightZoomRatio = 3;
    } else if (height <= kXrandrHeight_624){
        m_heightZoomRatio = 2;
    } else if (height <= kXrandrHeight_864){
        m_heightZoomRatio = 1.5;
    }
}


installer::ScreenAdaptationManager::ScreenAdaptationManager()
{
    init();
}

