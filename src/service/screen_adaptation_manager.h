#ifndef SCREEN_ADAPTATION_MANAGER_H
#define SCREEN_ADAPTATION_MANAGER_H

#include <QString>
#include <QPixmap>

namespace installer {

class ScreenAdaptationManager : public QObject
{
    Q_OBJECT
public:
    static ScreenAdaptationManager* instance();

public:
    QPixmap adapterPixmap(const QString &file);
    QPixmap adapterPixmap(const QPixmap &pixmap);
    QRect primaryGeometry();
    QRect primaryAvailableGeometry();
    int getMainWindowTopMargin();
    int getChildWindowTopMargin();

Q_SIGNALS:
    void screenGeometryChanged(const QRect &rect);
    void primaryGeometryChanged(const QRect &rect);
    void primaryAvailableGetometryChanaged(const QRect &rect);

private:
    void setup(const QRect &rect);
    void initConnection();
    void adapterWidth(int width);
    void adapterHeight(int height);

private:
    ScreenAdaptationManager();
    ScreenAdaptationManager(const ScreenAdaptationManager &) = delete;

private:
    double      m_widthZoomRatio = 1;
    double      m_heightZoomRatio = 1;
    QRect       m_oldWindowRect;
    QRect       m_oldAvailabWindowRect;
};


}

#endif // SCREEN_ADAPTATION_MANAGER_H
