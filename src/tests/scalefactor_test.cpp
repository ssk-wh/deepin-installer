#include <list>

#include "base/utils.h"
#include "third_party/googletest/include/gtest/gtest.h"
#include <QDebug>

namespace installer {
namespace {
TEST(Xrandr, ScaleFactor)
{
    struct Infos {
        int    width;
        int    height;
        int    widthMm;
        int    heightMm;
        double factor;
    };

    std::list<Infos> list{
        { 1366, 768, 310, 147, 1 },     { 1366, 768, 277, 165, 1 },
        { 1366, 768, 309, 174, 1 },     { 1600, 900, 294, 166, 1 },
        { 1920, 1080, 344, 194, 1.25 }, { 1920, 1080, 477, 268, 1 },
        { 1920, 1080, 527, 296, 1 },    { 1920, 1080, 476, 268, 1 },
        { 1920, 1080, 520, 310, 1 },    { 1920, 1080, 708, 398, 1 },
        { 1920, 1080, 518, 324, 1 },    { 1920, 1080, 510, 287, 1 },
        { 1920, 1080, 527, 296, 1 },    { 1920, 1080, 309, 174, 1.25 },
        { 1920, 1080, 293, 165, 1.25 }, { 1920, 1080, 294, 165, 1.25 },
        { 2160, 1440, 280, 180, 1.5 },  { 3000, 2000, 290, 200, 2 },
        { 3840, 2160, 600, 340, 2 },    { 3840, 2160, 344, 193, 2.25 },
    };

    for (const Infos& testInfo : list) {
        auto factor = Utils::scaleFactor(testInfo.width, testInfo.height,
                                         testInfo.widthMm, testInfo.heightMm);
                                         qDebug() << factor << testInfo.width << testInfo.height;
        ASSERT_EQ(factor, testInfo.factor);
    }
};
}  // namespace
}  // namespace installer
