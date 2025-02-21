/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCSSFONTINFO_H
#define KOCSSFONTINFO_H

#include <QFont>
#include <QMap>
#include <kritaflake_export.h>
#include <boost/operators.hpp>

/**
 * @brief The KoCSSFontInfo class
 * Convenience struct to make it easier to use KoFontRegistery.
 * This struct contains default css values and can generate the axisvalues
 * for the given values.
 */

struct KRITAFLAKE_EXPORT KoCSSFontInfo: public boost::equality_comparable<KoCSSFontInfo> {

    QStringList families;

    double size = -1; /// < Size in Pt.
    bool automaticOpticalSizing = true;
    double fontSizeAdjust = 1.0;
    double weight = 400;
    double width = 100;

    QFont::Style slantMode = QFont::StyleNormal;
    bool autoSlant = true;
    double slantValue = 0.0;

    QMap<QString, double> axisSettings;

    QMap<QString, double> computedAxisSettings() const {
        QMap<QString, double> settings;
        settings.insert("wght", weight);
        settings.insert("wdth", width);
        if (automaticOpticalSizing) {
            settings.insert("opsz", size);
        }
        if (slantMode == QFont::StyleItalic) {
            settings.insert("ital", 1);
        } else if (slantMode == QFont::StyleOblique) {
            settings.insert("slnt", -(autoSlant? 14.0: slantValue));
        } else {
            settings.insert("ital", 0);
        }
        for (auto it = axisSettings.begin(); it != axisSettings.end(); it++) {
            settings.insert(it.key(), it.value());
        }

        return settings;
    }

    bool operator==(const KoCSSFontInfo &rhs) const {
        bool sizeMatch = automaticOpticalSizing? true: size == rhs.size;
        bool slantMatch = autoSlant == rhs.autoSlant? true: slantValue == rhs.slantValue;
        return families == rhs.families
                && automaticOpticalSizing == rhs.automaticOpticalSizing
                && sizeMatch
                && weight == rhs.weight
                && width == rhs.width
                && slantMode == rhs.slantMode
                && slantMatch
                && axisSettings == rhs.axisSettings;
    }
};

#endif // KOCSSFONTINFO_H
