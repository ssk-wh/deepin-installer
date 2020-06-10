#include "base/utils.h"
#include "base/file_util.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include <QString>
#include <QDebug>

namespace Utils
{

void AutoScreenScale()
{
    const QString& result = installer::ReadFile("/proc/cmdline");
    if (result.contains("installer.disable.scale")) {
        return;
    }

    double scaleRatio = 1.0;
    Display *display = XOpenDisplay(NULL);

    XRRScreenResources *resources = XRRGetScreenResourcesCurrent(display, DefaultRootWindow(display));

    if (!resources) {
        resources = XRRGetScreenResources(display, DefaultRootWindow(display));
        qWarning() << "get XRRGetScreenResourcesCurrent failed, use XRRGetScreenResources.";
    }

    if (resources) {
        std::list<double> scaleRatioList;
        for (int i = 0; i < resources->noutput; i++)
        {
            XRROutputInfo *outputInfo = XRRGetOutputInfo(display, resources, resources->outputs[i]);
            if (outputInfo == nullptr) {
                continue;
            }

            if (outputInfo->crtc == 0 || outputInfo->mm_width == 0)
                continue;

            XRRCrtcInfo *crtInfo = XRRGetCrtcInfo(display, resources, outputInfo->crtc);
            if (crtInfo == nullptr) {
                continue;
            }

            scaleRatio = scaleFactor(crtInfo->width, crtInfo->height, outputInfo->mm_width, outputInfo->mm_height);
        }

        if (!scaleRatioList.empty()) {
            scaleRatioList.sort();
            scaleRatio = *scaleRatioList.begin();
        }

        XRRFreeScreenResources(resources);
    }

    qDebug() << "scale factor: " << QString::number(scaleRatio);
    qputenv("QT_SCALE_FACTOR", QString::number(scaleRatio).toUtf8());
}
} // namespace Utils
