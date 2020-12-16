/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef COLORSFILTERS_H
#define COLORSFILTERS_H

#include <QObject>
#include <QVariant>
#include "kis_perchannel_filter.h"
#include "filter/kis_color_transformation_filter.h"


class ColorsFilters : public QObject
{
    Q_OBJECT
public:
    ColorsFilters(QObject *parent, const QVariantList &);
    ~ColorsFilters() override;
};

class KisAutoContrast : public KisFilter
{
public:
    KisAutoContrast();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("autocontrast", i18n("Auto Contrast"));
    }

};


#endif
