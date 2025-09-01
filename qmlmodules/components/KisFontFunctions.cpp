/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisFontFunctions.h"
#include <KoWritingSystemUtils.h>
#include <KoFontRegistry.h>

QString KisFontFunctions::wwsFontFamilyName(QString familyName)
{
    std::optional<QString> name = KoFontRegistry::instance()->wwsNameByFamilyName(familyName);
    if (!name) {
        return familyName;
    }
    return name.value();
}

QVariant KisFontFunctions::wwsFontFamilyNameVariant(QString familyName)
{
    std::optional<QString> name = KoFontRegistry::instance()->wwsNameByFamilyName(familyName);
    if (!name) {
        return QVariant();
    }
    return name.value();
}

QString KisFontFunctions::sampleTagForQLocale(const QLocale &locale)
{
    return KoWritingSystemUtils::sampleTagForQLocale(locale);
}

QVariantMap KisFontFunctions::getMapFromQVariant(QVariant var)
{
    return var.toMap();
}
