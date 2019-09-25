#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QString>
#include <map>

namespace installer {
enum class TranslatorType {
    SelectLanguageSubTitle,
};
}  // namespace installer

static const std::map<installer::TranslatorType, QString> TS_MAP{
    { installer::TranslatorType::SelectLanguageSubTitle, QObject::tr("Select system language") },
};

#endif
