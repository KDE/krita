/*
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef _KIS_WS_ENGINE_FILTER_H_
#define _KIS_WS_ENGINE_FILTER_H_

#include <kdebug.h>

#include <kis_view.h>
#include <kis_filter.h>
#include <kis_id.h>

class KisWSEngineFilterConfiguration : public KisFilterConfiguration
{

public:

    KisWSEngineFilterConfiguration() { m_pixels = 10000; }

    KisWSEngineFilterConfiguration(Q_UINT32 pixels = 0) { m_pixels = pixels; }

    Q_UINT32 pixels() { return m_pixels; }

private:

    Q_UINT32 m_pixels; // The number of pixels the filter should
               // move. 0 means keep running indefinitely



};

class KisWSEngineFilter : public KisFilter
{

public:

    KisWSEngineFilter();

    virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* cfg, const QRect& rc);

    static inline KisID id() { return KisID("Wet & Sticky Engine", i18n("Wet & Sticky")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return false; }
    virtual bool supportsIncrementalPainting() { return false; }

public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceSP dev);


private:

private:

    KisWSEngineFilterConfiguration * m_cfg;
    KisPaintDeviceSP m_src;
    KisPaintDeviceSP m_dst;
    QRect m_rect;

};

#endif // _KIS_WS_ENGINE_FILTER_H_
