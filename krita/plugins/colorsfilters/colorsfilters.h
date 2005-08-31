/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef BRIGHTNESSCONTRAST_H
#define BRIGHTNESSCONTRAST_H

#include <kparts/plugin.h>
#include "kis_perchannel_filter.h"

class KisDesaturateFilter : public KisFilter {
public:
    KisDesaturateFilter();
public:
    virtual void process(KisPaintDeviceImplSP, KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("desaturate", i18n("Desaturate")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
};

class KisGammaCorrectionFilter : public KisDoublePerChannelFilter {
public:
    KisGammaCorrectionFilter();
public:
    virtual void process(KisPaintDeviceImplSP, KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("gammadjustment", i18n("Gamma Adjustment")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
};

class KisColorAdjustmentFilter : public KisIntegerPerChannelFilter {
public:
    KisColorAdjustmentFilter();
public:
    virtual void process(KisPaintDeviceImplSP, KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("coloradjustment", i18n("Color Adjustment")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }

};

class KisAutoContrast : public KisFilter {
public:
    KisAutoContrast();
public:
    virtual void process(KisPaintDeviceImplSP, KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("autocontrast", i18n("Auto Contrast")); };
    virtual bool supportsPreview() { return true; }
    virtual bool supportsPainting() { return false; }

};

class ColorsFilters : public KParts::Plugin
{
public:
    ColorsFilters(QObject *parent, const char *name, const QStringList &);
    virtual ~ColorsFilters();

private:
    KisPainter *m_painter;
};

#endif
