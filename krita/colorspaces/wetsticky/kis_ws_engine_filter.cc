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
#include <stdlib.h>
#include <vector>
#include <math.h>

#include <QPoint>
#include <QSpinBox>
#include <QRect>
#include <QColor>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_debug_areas.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_debug_areas.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_colorspace_registry.h>

#include "kis_ws_engine_filter.h"
#include "kis_wet_sticky_colorspace.h"

/**
 * The Wet & Sticky Engine filter is based on the wet & sticky model
 * for computer painting designed by Tunde Cockshott and implemented
 * by David England and Kevin Waite.
 *
 * The filter implements the engine that moves the paint according to
 * gravity, viscosity and absorbency.
 *
 */
KisWSEngineFilter::KisWSEngineFilter() : KisFilter(id(), "", i18n("&Wet & Sticky paint engine..."))
{
}


/**
 * Sets the POINT giving the coordinate location of the next
 * cell on the canvas to be visited.  There is an even probability
 * of each cell being visited.
 */
QPoint next_cell(quint32 width, quint32 height)
{
    return QPoint(random() * width,  random() * height);
}

void single_step(KisColorSpace * cs, KisPaintDeviceSP src,  KisPaintDeviceSP dst, const QRect & rect, bool native)
{
    using namespace WetAndSticky;


    QPoint p = next_cell( rect.width(),  rect.height() );

    // XXX: We could optimize by randomly doing lines of 64 pixels
    // -- maybe that would be enough to avoid the windscreen wiper
    // effect.
    KisHLineIterator iter = src -> createHLineIterator(p.x(), p.y(), 1,  false);

    quint8 *orig = iter.rawData();
    quint8 *pix = orig;

     if (!orig) return;

    if (!native ) {
        QColor c;
        quint8 opacity;

        src -> colorSpace() -> toQColor(pix, &c, &opacity);
        quint8 *pix = new quint8[sizeof( cell )];
        Q_CHECK_PTR(pix);

        cs -> fromQColor(c, opacity, pix);
    }

    // Process

    CELL_PTR c = ( CELL_PTR )pix;


    if ( !native ) {
        // Set RGBA back
    }

}

void KisWSEngineFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{

    m_src = src;
    m_dst = dst;
    m_cfg = ( KisWSEngineFilterConfiguration * )configuration;
    m_rect = rect;


    kDebug(DBG_AREA_FILTERS) << "WSEnginefilter called!\n";
    QTime t;
    t.restart();

    // Two possibilities: we have our own, cool w&s pixel, and
    // then we have real data to mess with, or we're filtering a
    // boring shoup-model paint device and we can only work by
    // synthesizing w&s pixels.
    bool native = false;
    // XXX: We need a better way to ID color strategies
    if ( src -> colorSpace() -> id() == KisID("W&S","") ) native = true;

    // XXX: We need a better way to ID color strategies
    KisColorSpace * cs = KisColorSpaceRegistry::instance()->get("W&S");

    quint32 pixels = 400; //m_cfg -> pixels();

    kDebug(DBG_AREA_FILTERS) << "Going to singlestep " << pixels << " pixels.\n";

    // Determine whether we want an infinite loop
    if ( pixels == 0 ) {
        while ( true )
            single_step (cs, src, dst, rect, native);
    }
    // Or not.
    else {
        for ( quint32 i = 0; i < pixels; ++i ) {
            single_step (cs, src, dst, rect, native);
        }
    }
    kDebug(DBG_AREA_FILTERS) << "Done in " << t.elapsed() << " ms\n";

}

KisFilterConfigWidget * KisWSEngineFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev)
{
//     KisWSEngineFilterConfigurationWidget* kefcw = new KisWSEngineFilterConfigurationWidget(this,parent, "");
//     kDebug(DBG_AREA_FILTERS) << kefcw << endl;
//     return kefcw  ;
    return 0;
}

KisFilterConfiguration* KisWSEngineFilter::configuration(QWidget* nwidget, KisPaintDeviceSP dev)
{
//     KisWSEngineFilterConfigurationWidget* widget = (KisWSEngineFilterConfigurationWidget*) nwidget;

//     if( widget == 0 )
//     {
//         return new KisWSEngineFilterConfiguration(30);
//     } else {
//                 quint32 depth = widget -> baseWidget() -> depthSpinBox -> value();

//                 return new KisWSEngineFilterConfiguration(depth);
//         }


    return new KisWSEngineFilterConfiguration( m_rect.height() * m_rect.width() );
}

