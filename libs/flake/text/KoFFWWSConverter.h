/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFFWWSCONVERTER_H
#define KOFFWWSCONVERTER_H

#include <QHash>
#include <KoFontLibraryResourceUtils.h>

struct KoFontFamilyAxis {

    KoFontFamilyAxis() {}
    KoFontFamilyAxis (QString _tag, qreal _value)
        : tag(_tag), value(_value) {}

    static KoFontFamilyAxis weightAxis(qreal val) {
        KoFontFamilyAxis axis("wght", val);
        axis.min = 0;
        axis.max = 1000;
        return axis;
    }
    static KoFontFamilyAxis widthAxis(qreal val) {
        KoFontFamilyAxis axis("wdth", val);
        axis.min = 0;
        axis.max = 200;
        return axis;
    }
    static KoFontFamilyAxis slantAxis(qreal val) {
        KoFontFamilyAxis axis("slnt", val);
        axis.min = -90.0;
        axis.max = 90.0;
        return axis;
    }

    QString tag;
    QHash<QString, QString> localizedLabels;
    qreal min = -1;
    qreal max = -1;
    qreal value = 0;
};

struct FontFamilySizeInfo {
    bool os2table = false;
    int subFamilyID = 0;

    qreal low = -1;
    qreal high = -1;
    qreal designSize = 0;
    QHash<QString, QString> localizedLabels;
};

struct KoFontFamilyNode {
    QString fontFamily;
    QString fontStyle;
    QString fileName;
    int fileIndex = 0;

    QHash<QString, QString> localizedFontFamilies;
    QHash<QString, QString> localizedFontStyle;

    QHash<QString, KoFontFamilyAxis> axes;

    QHash<int, QString> pixelSizes;
    FontFamilySizeInfo sizeInfo;

    bool isItalic = false;
    bool isOblique = false;

    QString debugInfo();
};

class KoFFWWSConverter
{
public:
    KoFFWWSConverter();
    ~KoFFWWSConverter();

    bool addFontFromPattern(const FcPattern *pattern, FT_LibrarySP freeTypeLibrary);

    void debugInfo();
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFFWWSCONVERTER_H
