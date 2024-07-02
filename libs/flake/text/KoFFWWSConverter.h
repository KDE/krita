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
        axis.min = val;
        axis.max = val;
        axis.defaultValue = 400;
        return axis;
    }
    static KoFontFamilyAxis widthAxis(qreal val) {
        KoFontFamilyAxis axis("wdth", val);
        axis.min = val;
        axis.max = val;
        axis.defaultValue = 100;
        return axis;
    }
    static KoFontFamilyAxis slantAxis(qreal val) {
        KoFontFamilyAxis axis("slnt", val);
        axis.min = val;
        axis.max = val;
        axis.defaultValue = 0;
        return axis;
    }

    QString tag;
    QHash<QString, QString> localizedLabels;
    qreal min = -1;
    qreal max = -1;
    qreal value = 0;
    qreal defaultValue = 0;
    bool variableAxis = false;

    QString debugInfo() const {
        QString label;
        if (!localizedLabels.isEmpty()) {
            label = localizedLabels.value("en", localizedLabels.values().first());
        }
        return QString("Axis: %1 (%2), min: %3, default:%4, max: %5").arg(tag).arg(label).arg(min).arg(value).arg(max);
    }
};

struct FontFamilySizeInfo {
    bool os2table = false;
    int subFamilyID = 0;

    qreal low = -1;
    qreal high = -1;
    qreal designSize = 0;
    QHash<QString, QString> localizedLabels;
    QString debugInfo() const {
        QString label;
        if (!localizedLabels.isEmpty()) {
            label = localizedLabels.value("en", localizedLabels.values().first());
        }
        return QString("Optical Size Info: OS2=%1, label: %2, min: %3, max: %4, designSize: %5").arg(os2table? "true": "false").arg(label).arg(low).arg(high).arg(designSize);
    }
};

enum FontFormatType {
    Unknown,
    BDF,
    Type1,
    OpenType
};

struct FontFamilyStyleInfo {
    QHash<QString, QString> localizedLabels;
    QHash<QString, float> instanceCoords;

    QString debugInfo() const {
        QString label;
        if (!localizedLabels.isEmpty()) {
            label = localizedLabels.value("en", localizedLabels.values().first());
        }
        QStringList coords;
        for (int i = 0; i < instanceCoords.size(); i++) {
            QString key = instanceCoords.keys().at(i);
            coords.append(key+"="+QString::number(instanceCoords.value(key)));
        }
        return QString("Instance: %1, coords: [ %2 ]").arg(label).arg(coords.join(" "));
    }
};

struct KoFontFamilyNode {
    QString fontFamily;
    QString fontStyle;
    QString fileName;
    int fileIndex = 0;

    QHash<QString, QString> localizedFontFamilies;
    QHash<QString, QString> localizedFontStyle;

    QHash<QString, KoFontFamilyAxis> axes;
    QList<FontFamilyStyleInfo> styleInfo;

    QHash<int, QStringList> pixelSizes;
    FontFamilySizeInfo sizeInfo;

    bool isItalic = false;
    bool isOblique = false;

    bool compareAxes(QHash<QString, KoFontFamilyAxis> otherAxes) {
        if (axes.keys() != otherAxes.keys()) {
            return false;
        }
        for (int k = 0; k < axes.keys().size(); k++) {
            KoFontFamilyAxis a = axes.value(axes.keys().at(k));
            KoFontFamilyAxis b = otherAxes.value(axes.keys().at(k));
            if (a.value != b.value) {
                return false;
            }
        }
        return true;
    }

    FontFormatType type = Unknown;
    bool isVariable = false;
    bool colorClrV0 = false;
    bool colorClrV1 = false;
    bool colorSVG = false;
    bool colorBitMap = false;

    QStringList debugInfo();
};

class KoFFWWSConverter
{
public:
    KoFFWWSConverter();
    ~KoFFWWSConverter();

    bool addFontFromPattern(const FcPattern *pattern, FT_LibrarySP freeTypeLibrary);

    void sortIntoWWSFamilies();

    void debugInfo();
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFFWWSCONVERTER_H
