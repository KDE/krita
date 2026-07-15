/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */
#ifndef KISTONEGAMUTMAPFILTER_H
#define KISTONEGAMUTMAPFILTER_H

#include "filter/kis_color_transformation_filter.h"

class KisToneGamutMapFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisToneGamutMapFilterPlugin(QObject *parent, const QVariantList &);
    ~KisToneGamutMapFilterPlugin() override;
};

class KisToneGamutMapFilter: public KisColorTransformationFilter
{
public:
    KisToneGamutMapFilter();

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("tone_gamut_map", i18n("Tone/Gamut Map"));
    }

};

#endif // KISTONEGAMUTMAPFILTER_H
