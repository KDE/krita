/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFFWWSCONVERTER_H
#define KOFFWWSCONVERTER_H

#include <QHash>
#include <KoFontLibraryResourceUtils.h>
#include "KoCSSFontInfo.h"
#include <kritaflake_export.h>

#include <KoSvgText.h>
#include <QLocale>
#include <QDateTime>

#include <optional>

/// This struct represents a CSS-compatible font family, containing all
/// sorts of info useful for the GUI.
struct KoFontFamilyWWSRepresentation {
    QString fontFamilyName;
    QString typographicFamilyName;

    QHash<QLocale, QString> localizedFontFamilyNames;
    QHash<QLocale, QString> localizedTypographicFamily;
    QHash<QLocale, QString> localizedTypographicStyles;

    QDateTime lastModified; ///< Value of the most recently modified font family. Used for updates.

    QHash<QString, QString> sampleStrings; /// sample string used to generate the preview;
    QList<QLocale> supportedLanguages;

    QHash<QString, KoSvgText::FontFamilyAxis> axes;
    QList<KoSvgText::FontFamilyStyleInfo> styles;

    KoSvgText::FontFormatType type = KoSvgText::UnknownFontType;
    bool isVariable = false;
    bool colorClrV0 = false;
    bool colorClrV1 = false;
    bool colorSVG = false;
    bool colorBitMap = false;
};


/**
 * @brief The KoFFWWSConverter class
 * This class takes fontconfig patterns and tries to sort them into
 * a hierarchy of typographic/wws/font files font families, as well
 * as retrieving all sorts of information to display in the GUI.
 *
 * This is necessary because by default FontConfig patterns don't
 * differentiate between fonts family names, which means that some
 * fonts are not selectable by CSS values alone.
 */
class KRITAFLAKE_EXPORT KoFFWWSConverter
{
public:
    KoFFWWSConverter();
    ~KoFFWWSConverter();

    struct FontFileEntry {
        QString fileName;
        int fontIndex;
    };

    /// Add a font from a fontconfig pattern.
    bool addFontFromPattern(const FcPattern *pattern, FT_LibrarySP freeTypeLibrary);

    /// Add a font from a filename and index.
    /// This will use freetype and harfbuzz to figure out the family name(s), styles
    /// and other font features.
    bool addFontFromFile(const QString &filename, const int index, FT_LibrarySP freeTypeLibrary);

    void addSupportedLanguagesByFile(const QString &filename, const int index, const QList<QLocale> &supportedLanguages, FcCharSet *set);

    /// Sort any straggling fonts into WWSFamilies.
    void sortIntoWWSFamilies();

    /// This adds a CSS generic family. Call this before sortIntoWWSFamilies.
    void addGenericFamily(const QString &name);

    /// Collects all WWSFamilies (that is, CSS compatible groupings of font files) and return them.
    QList<KoFontFamilyWWSRepresentation> collectFamilies() const;

    /// Gets a single WWSFamily representation for a given CSS Family Name, used by KoFontStorage.
    std::optional<KoFontFamilyWWSRepresentation> representationByFamilyName(const QString &familyName) const;

    /// Used to find the closest corresponding resource when the family name doesn't match.
    std::optional<QString> wwsNameByFamilyName(const QString familyName) const;

    /**
     * @brief candidatesForCssValues
     * Search the nodes for the most appropriate font for the given css values.
     * We want to give these preferential treatment to whatever fontconfig matches for us.
     * @return list of QPairs representing the filenames and file indices for the candidates.
     */
    QVector<FontFileEntry> candidatesForCssValues(const KoCSSFontInfo info,
                                       quint32 xRes = 72,
                                       quint32 yRes = 72) const;

    /// Print out the font family hierarchy into the debug output.
    void debugInfo() const;

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFFWWSCONVERTER_H
