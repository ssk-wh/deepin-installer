#pragma once

#include <cmath>
#include <functional>
#include <QString>

namespace Utils {

double toListedScaleFactor(double s)
{
    const double min  = 1.0;
    const double max  = 3.0;
    const double step = 0.25;

    if (s <= min) {
        return min;
    }
    else if (s >= max) {
        return max;
    }

    for (double i = min; i <= max; i += step) {
        if (i > s) {
            auto ii = i - step;
            auto d1 = s - ii;
            auto d2 = i - s;
            return d1 >= d2 ? i : ii;
        }
    }

    return max;
}

double scaleFactor(uint widthPx, uint heightPx, uint widthMm, uint heightMm)
{
    double lenPx    = hypot(widthPx, heightPx);
    double lenMm    = hypot(widthMm, heightMm);
    double lenPxStd = hypot(1920, 1080);
    double lenMmStd = hypot(477, 268);
    double a        = 0.00158;
    double fix      = (lenMm - lenMmStd) * (lenPx / lenPxStd) * a;

    return toListedScaleFactor((lenPx / lenMm) / (lenPxStd / lenMmStd) + fix);
};

template <typename T>
void addTransLate(T &t, std::function<void (QString)> function, const QString &tr) {
    t.push_back(std::pair<std::function<void (QString)>, QString>(function, tr));
}

}  // namespace Utils
