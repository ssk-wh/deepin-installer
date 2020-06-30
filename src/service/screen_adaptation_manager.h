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

private:
    virtual void init();

private:
    ScreenAdaptationManager();
    ScreenAdaptationManager(const ScreenAdaptationManager &) = delete;

private:
    double m_zoomRatio = 1;
};


}

#endif // SCREEN_ADAPTATION_MANAGER_H
