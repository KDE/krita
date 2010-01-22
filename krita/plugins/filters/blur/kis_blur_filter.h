/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_BLUR_FILTER_H
#define KIS_BLUR_FILTER_H

#include "filter/kis_filter.h"

class KisBlurFilter : public KisFilter
{
public:
    KisBlurFilter();
public:
    using KisFilter::process;

    void process(KisConstProcessingInformation src,
                 KisProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
                ) const;
    static inline KoID id() {
        return KoID("blur", i18n("Blur"));
    }

    virtual KisFilterConfiguration* factoryConfiguration(const KisPaintDeviceSP) const;
public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image = 0) const;
    QRect neededRect(const QRect & rect, const KisFilterConfiguration*) const;
    QRect changedRect(const QRect & rect, const KisFilterConfiguration*) const;
};

#endif
