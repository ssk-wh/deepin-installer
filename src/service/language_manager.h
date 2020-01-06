/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QWidget>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "base/translator.h"

namespace installer {

class ObjectMapper {
private:
    friend class LanguageManager;
    QObject*                            target;
    bool                                isLife = false;
    std::function<void(const QString&)> func;
    installer::TranslatorType           type;
};

class LanguageManager : public QObject {
    Q_OBJECT
private:
    LanguageManager();

    bool eventFilter(QObject* watched, QEvent* event) override;

public:
    static LanguageManager* Instance();

    template <typename T,
              typename Func,
              typename = std::enable_if<std::is_base_of<QWidget, T>::value>>
    static std::shared_ptr<ObjectMapper> translator(T                         sender,
                                                    Func                      fun,
                                                    installer::TranslatorType type)
    {
        LanguageManager* instance{ LanguageManager::Instance() };

        std::shared_ptr<ObjectMapper> ob(new ObjectMapper);
        ob->func   = std::bind(fun, sender, std::placeholders::_1);
        ob->type   = type;
        ob->isLife = true;
        ob->target = sender;

        return instance->indexOf(instance->add(ob));
    }

    void                          remove(std::shared_ptr<ObjectMapper> obj);
    size_t                        indexOf(std::shared_ptr<ObjectMapper> obj);
    std::shared_ptr<ObjectMapper> indexOf(size_t index);

    static QString tsText(installer::TranslatorType type);

  private:
    size_t add(std::shared_ptr<ObjectMapper> obj);

private:
    std::vector<std::shared_ptr<ObjectMapper>>       trList;
};
}  // namespace installer

#endif
