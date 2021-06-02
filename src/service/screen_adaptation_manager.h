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
    QRect primaryAvailableGeometry();
    int getMainWindowTopMargin();
    int getChildWindowTopMargin();
    int adapterHeightMargin(int height, int Margin = 55);
    bool is4KScreen();

    double getWidthZoomRatio() const;
    double getHeightZoomRatio() const;

Q_SIGNALS:
    void screenGeometryChanged(const QRect &rect);
    void primaryGeometryChanged(const QRect &rect);
    void primaryAvailableGetometryChanaged(const QRect &rect);

private:
    void setup(const QRect &rect);
    void initConnection();
    void adapterWidth(int width);
    void adapterHeight(int height);
    void getCurrentScreenResolution();

private:
    ScreenAdaptationManager();
    ScreenAdaptationManager(const ScreenAdaptationManager &) = delete;

private:
    double      m_widthZoomRatio = 1;
    double      m_heightZoomRatio = 1;
    QRect       m_oldWindowRect;
    QRect       m_oldAvailabWindowRect;
    QPoint      m_currentScreenResolution;
};


}

#endif // SCREEN_ADAPTATION_MANAGER_H
