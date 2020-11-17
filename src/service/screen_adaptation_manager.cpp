#include "screen_adaptation_manager.h"

#include <DApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QDebug>
#include "settings_manager.h"
#include "settings_name.h"

DWIDGET_USE_NAMESPACE

namespace  {
    const int kXrandrWidth_832 = 832;
    const int kXrandrWidth_1024 = 1024;

    const int kXrandrHeight_624 = 624;
    const int kXrandrHeight_864 = 864;
    const int kXrandrHeight_480 = 480;
    const int XRANDR_INVALID = 999999;
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

void installer::ScreenAdaptationManager::adapterWindows(QWidget *widget)
{
    widget->setFixedSize(adapterScreenGeometry().size());
    widget->move(QPoint(adapterScreenGeometry().x(), adapterScreenGeometry().y()));
//    widget->setGeometry(adapterScreenGeometry());
}


void installer::ScreenAdaptationManager::init()
{
    QRect rect = adapterScreenGeometry();

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
        m_widthZoomRatio = 2.5;
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

int installer::ScreenAdaptationManager::getXRandrWidth()
{
    return getXRandrWidth(m_adaptationResolution);
}

int installer::ScreenAdaptationManager::getXRandrWidth(const QString &xrandrs)
{
    QStringList xrandrlist = xrandrs.split("x");
    if (xrandrlist.size() != 2) {
        return XRANDR_INVALID;
    }

    bool ok = false;
    int res = xrandrlist.at(0).toInt(&ok);
    if (ok) {
        return res;
    } else {
        return XRANDR_INVALID;
    }
}

int installer::ScreenAdaptationManager::getXRandrHeight()
{
    return getXRandrHeight(m_adaptationResolution);
}

int installer::ScreenAdaptationManager::getXRandrHeight(const QString &xrandrs)
{
    QStringList xrandrlist = xrandrs.split("x");
    if (xrandrlist.size() != 2) {
        return XRANDR_INVALID;
    }

    bool ok = false;
    int res = xrandrlist.at(1).toInt(&ok);
    if (ok) {
        return res;
    } else {
        return XRANDR_INVALID;
    }
}

bool installer::ScreenAdaptationManager::isAdapterWindows()
{
    return getXRandrWidth() < QApplication::desktop()->width()
            && getXRandrHeight() < QApplication::desktop()->height();
}

QRect installer::ScreenAdaptationManager::adapterScreenGeometry()
{
    if (isAdapterWindows()) {
        QPoint pos = QPoint((QApplication::desktop()->width() - getXRandrWidth()) / 2,
                (QApplication::desktop()->height() - getXRandrHeight()) / 2);
        return QRect(pos, QSize(getXRandrWidth(), getXRandrHeight()));

    } else {
        return QApplication::desktop()->screenGeometry();
    }
}

installer::ScreenAdaptationManager::ScreenAdaptationManager():
    m_adaptationResolution(GetSettingsString(kWindowAdaptationResolution))
{
    init();
}

