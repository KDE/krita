/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MINMAX_FILTERS_H
#define KIS_MINMAX_FILTERS_H

#include "filter/kis_filter.h"

class KisFilterMax : public KisFilter
{
public:

    KisFilterMax();

    void processImpl(KisPaintDeviceSP src,
                     const QRect& size,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;

    static inline KoID id() {
        return KoID("maximize", i18n("Maximize Channel"));
    }

};

class KisFilterMin : public KisFilter
{
public:
    KisFilterMin();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& rect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("minimize", i18n("Minimize Channel"));
    }
};

#endif
