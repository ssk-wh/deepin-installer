#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QString>
#include <map>

namespace installer {
enum class TranslatorType {
    NextButton,
    BackButton,
    SelectLanguageSubTitle,
};
}  // namespace installer

static const std::map<installer::TranslatorType, QString> TS_MAP{
    { installer::TranslatorType::NextButton, ::QObject::tr("Next") },
    { installer::TranslatorType::BackButton, ::QObject::tr("Back", "button") },
    { installer::TranslatorType::SelectLanguageSubTitle, ::QObject::tr("Select system language") },
};

#endif
