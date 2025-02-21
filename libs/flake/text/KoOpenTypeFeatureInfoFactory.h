/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <QString>
#include <QStringList>
#include <QVector>
#include <QScopedPointer>

/*
 * KoOpenTypeFeatureInfo provides basic info about
 * each opentype feature, with translatable strings.
 **/
struct KoOpenTypeFeatureInfo
{
    enum OpenTypeTable {
        GSUB1, ///< Single substitution.
        GSUB2, ///< Multiple substitution (1 -> 2)
        GSUB3, ///< Alternate substitution
        GSUB4, ///< Ligature substitution.
        GSUB5, ///< Contextual substitution.
        GSUB6, ///< Chained Contextual substitution.
        GSUB7, ///< Extension substitution.
        GSUB8, ///< Reverse Chained Contextual substitution.
        GPOS1, ///< Single position adjustment.
        GPOS2, ///< Pair position adjustment.
        GPOS3, ///< Cursive attachment.
        GPOS4, ///< Mark-to-base adjustment.
        GPOS5, ///< Mark-to-ligature adjustment.
        GPOS6, ///< Mark-to-mark adjustment.
        GPOS7, ///< Contextual positioning.
        GPOS8, ///< Chained-Contextual positioning.
        GPOS9, ///< Extension positioning.
    };
    KoOpenTypeFeatureInfo() {}
    KoOpenTypeFeatureInfo(const QLatin1String &tag,
                          const QString &name,
                          const QString &description,
                          const QVector<OpenTypeTable> &tables,
                          const bool glyphPalette = false)
    : tag(tag)
    , name(name)
    , description(description)
    , tables(tables)
    , glyphPalette(glyphPalette)
    {
    }
    
    QLatin1String tag; ///< 4 char tag.
    
    QString name; ///< User-friendly name.
    
    QString description; ///< Description of the feature.

    QString sample; ///< Sample of the feature, if any. Only used by CVXX features and retrieved from the font.

    QStringList namedParameters;/// Named parameters. Only used by CVXX features and retrieved from the font.
    
    QVector<OpenTypeTable> tables; ///< Which table type(s) are recommended for this feature in the official registery.
    
    bool glyphPalette{false}; ///< Whether the feature should be visible in the glyph palette.
    
};

/**
 * @brief The KoOpenTypeFeatureInfoFactory class
 *
 * This returns KoOpenTypeFeatureInfo's for the given tag.
 */

class KoOpenTypeFeatureInfoFactory
{
public:
    KoOpenTypeFeatureInfoFactory();
    ~KoOpenTypeFeatureInfoFactory();
    
    KoOpenTypeFeatureInfo infoByTag(const QLatin1String &tag);

private:
    struct Private;
    
    QScopedPointer<Private> d;
};
