#include "language_manager.h"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QPointer>
#include <QTimer>
#include <map>
#include <mutex>
#include <utility>

static QTimer*    tsLifeCycleTimer = new QTimer;
static std::mutex tsListMutex;

using namespace installer;

LanguageManager::LanguageManager()
{
    tsLifeCycleTimer->setInterval(0);
    tsLifeCycleTimer->setSingleShot(true);

    connect(tsLifeCycleTimer, &QTimer::timeout, this, [&] {
        std::unique_lock<std::mutex> lock(tsListMutex);

        for (auto it = trList.begin(); it != trList.end();) {
            std::shared_ptr<ObjectMapper> mapper = *it;
            if (mapper->isLife) {
                it = trList.erase(it);
            }
            else {
                ++it;
            }
        }
    });
}

bool LanguageManager::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == qApp && event->type() == QEvent::LanguageChange) {
        std::unique_lock<std::mutex> lock(tsListMutex);

        for (std::shared_ptr<ObjectMapper> mapper : trList) {
            if (mapper && qApp && mapper->target->parent()) {
                mapper->func(qApp->translate("QObject", TS_MAP.at(mapper->type).toUtf8()));
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

LanguageManager* LanguageManager::Instance()
{
    static LanguageManager manager;
    return &manager;
}

void LanguageManager::remove(std::shared_ptr<ObjectMapper> obj)
{
    // NOTE(justforlxz): 标记为销毁
    obj->isLife = false;

    tsLifeCycleTimer->start();
}

size_t LanguageManager::indexOf(std::shared_ptr<ObjectMapper> obj)
{
    for (auto it = trList.cbegin(); it != trList.cend(); ++it) {
        if (it->get() == obj.get()) {
            return static_cast<size_t>(it - trList.cbegin());
        }
    }

    return size_t(-1);
}

QString LanguageManager::tsText(installer::TranslatorType type) {
    return TS_MAP.at(type);
}

std::shared_ptr<ObjectMapper> LanguageManager::indexOf(size_t index)
{
    if (index > trList.size()) {
        return nullptr;
    }

    return trList[index];
}

size_t LanguageManager::add(std::shared_ptr<ObjectMapper> obj)
{
    trList.push_back(obj);

    obj->func(qApp->translate("QObject", TS_MAP.at(obj->type).toUtf8()));

    connect(obj->target, &QObject::destroyed, this, [=] { remove(obj); });

    return trList.size() - 1;
}
