#ifndef SCREEN_ADAPTATION_MANAGER_H
#define SCREEN_ADAPTATION_MANAGER_H

#include <QString>
#include <QPixmap>

namespace installer {

class ScreenAdaptationManager
{
public:
    static ScreenAdaptationManager* instance();

public:
    QPixmap adapterPixmap(const QString &file);
    QPixmap adapterPixmap(const QPixmap &pixmap);
    QRect adapterScreenGeometry();
    void adapterWindows(QWidget *wid);

private:
    virtual void init();
    virtual void adapterWidth(int width);
    virtual void adapterHeight(int height);

    int getXRandrWidth();
    int getXRandrWidth(const QString &xrandr);
    int getXRandrHeight();
    int getXRandrHeight(const QString &xrandr);
    bool isAdapterWindows();

private:
    ScreenAdaptationManager();
    ScreenAdaptationManager(const ScreenAdaptationManager &) = delete;

private:
    double m_widthZoomRatio = 1;
    double m_heightZoomRatio = 1;
    QString m_adaptationResolution;
};


}

#endif // SCREEN_ADAPTATION_MANAGER_H
