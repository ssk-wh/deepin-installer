#include "screen_adaptation_manager.h"

#include "settings_manager.h"
#include "settings_name.h"
#include "ui/widgets/wallpaper_item.h"
#include "base/command.h"

#include <DApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QDebug>
#include <QScreen>
#include <QThread>

DWIDGET_USE_NAMESPACE

namespace  {
    const int kXrandrWidth_832 = 832;
    const int kXrandrWidth_1024 = 1024;
    const int kXrandrWidth_1920 = 1920;
    const int kXrandrWidth_3840 = 3840;

    const int kXrandrHeight_624 = 624;
    const int kXrandrHeight_864 = 864;
    const int kXrandrHeight_480 = 480;
    const int kXrandrHeight_1080 = 1080;
    const int kXrandrHeight_2160 = 2160;
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
                         Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QRect installer::ScreenAdaptationManager::primaryAvailableGeometry()
{
    return QApplication::desktop()->availableGeometry();
}

int installer::ScreenAdaptationManager::getMainWindowTopMargin()
{
    qInfo() << "primary geometry:" << primaryAvailableGeometry();

    if (primaryAvailableGeometry().width() >= kXrandrWidth_1920
            && primaryAvailableGeometry().height() >= kXrandrHeight_1080) {
        return 90;
    }

    return 0;
}

int installer::ScreenAdaptationManager::getChildWindowTopMargin()
{
    if (primaryAvailableGeometry().width() >= kXrandrWidth_1920
            && primaryAvailableGeometry().height() >= kXrandrHeight_1080) {
        return 20;
    }

    return 0;
}

int installer::ScreenAdaptationManager::adapterHeightMargin(int height, int Margin)
{
    // 屏幕高度在700以上的， 按原始高度显示，否则为无法显示全的情况下对部分内容较少的控件进行缩放
    if (primaryAvailableGeometry().height() >= 700) {
        return height;
    }

    return height - Margin;
}

bool installer::ScreenAdaptationManager::is4KScreen()
{
    if (m_currentScreenResolution.x() >= kXrandrWidth_3840
            && m_currentScreenResolution.y() >= kXrandrHeight_2160) {
        return true;
    }

    return false;
}

double installer::ScreenAdaptationManager::getWidthZoomRatio() const
{
    return m_widthZoomRatio;
}

double installer::ScreenAdaptationManager::getHeightZoomRatio() const
{
    return m_heightZoomRatio;
}


void installer::ScreenAdaptationManager::setup(const QRect &rect)
{
    adapterWidth(rect.width());
    adapterHeight(rect.height());

    qDebug() << "rect.width() = " << rect.width();
    qDebug() << "rect.height() = " << rect.height();
    qDebug() << "rect() = " << rect;
    qDebug() << "m_widthZoomRatio = " << m_widthZoomRatio;
    qDebug() << "m_heightZoomRatio = " << m_heightZoomRatio;
}

void installer::ScreenAdaptationManager::initConnection()
{
    connect(QGuiApplication::primaryScreen(), &QScreen::availableGeometryChanged,
            this, &ScreenAdaptationManager::setup);
    connect(QGuiApplication::primaryScreen(), &QScreen::availableGeometryChanged,
            this, [=](const QRect &rect) {
        if (m_oldAvailabWindowRect != rect) {
            Q_EMIT primaryAvailableGetometryChanaged(rect);
        }

    });

    connect(QGuiApplication::primaryScreen(), &QScreen::geometryChanged,
            this, [=](const QRect &rect) {
        if (m_oldWindowRect != rect) {
            Q_EMIT primaryGeometryChanged(rect);
        }

    });
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

void installer::ScreenAdaptationManager::getCurrentScreenResolution()
{
    QString xrandrResult;
    qInfo() << SpawnCmd("xrandr", QStringList(), xrandrResult);

    QTextStream stream(&xrandrResult);
    QString line;
    while (stream.readLineInto(&line)) {
        if (line.contains("Screen") && line.contains("current")) {
            const QStringList list1 {
                line.simplified().split(",")
            };

            if (list1.count() > 2) {
                const QStringList list2 {
                    list1.at(1).simplified().split(" ")
                };

                if (list2.count() > 3) {
                    bool ok1, ok2;
                    int width = list2.at(1).toInt(&ok1);
                    int height = list2.at(3).toInt(&ok2);

                    if (ok1 && ok2) {
                        qInfo() << QString("Current screen resolution is:(%1, %2)").arg(width).arg(height);
                        m_currentScreenResolution = QPoint(width, height);

                        return;
                    }
                }
            }

            break;
        }
    }

    m_currentScreenResolution = QPoint();
}

installer::ScreenAdaptationManager::ScreenAdaptationManager()
{
    setup(primaryAvailableGeometry());
    initConnection();

    getCurrentScreenResolution();
}

