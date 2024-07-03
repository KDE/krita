/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFFWWSCONVERTER_H
#define KOFFWWSCONVERTER_H

#include <QHash>
#include <KoFontLibraryResourceUtils.h>
#include <kritaflake_export.h>

#include <KoSvgText.h>


struct KoFontFamilyWWSRepresentation {
    QString fontFamilyName;
    QString typographicFamilyName;

    QHash<QString, KoSvgText::FontFamilyAxis> axes;
    QList<KoSvgText::FontFamilyStyleInfo> styles;

    KoSvgText::FontFormatType type = KoSvgText::Unknown;
    bool isVariable = false;
    bool colorClrV0 = false;
    bool colorClrV1 = false;
    bool colorSVG = false;
    bool colorBitMap = false;
};

class KoFFWWSConverter
{
public:
    KoFFWWSConverter();
    ~KoFFWWSConverter();

    bool addFontFromPattern(const FcPattern *pattern, FT_LibrarySP freeTypeLibrary);

    void sortIntoWWSFamilies();

    QList<KoFontFamilyWWSRepresentation> collectFamilies() const;

    void debugInfo();
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFFWWSCONVERTER_H
