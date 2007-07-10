/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filterop.h"

#include <QRect>

#include <kdebug.h>

#include "KoColorSpace.h"
#include "KoCompositeOp.h"

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_config_widget.h"
#include "kis_filter_registry.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_paintop.h"
#include "kis_selection.h"

#include "ui_FilterOpOptionsWidget.h"

KisPaintOp * KisFilterOpFactory::createOp(const KisPaintOpSettings *_settings, KisPainter * _painter, KisImageSP _image)
{
    const KisFilterOpSettings* settings = dynamic_cast<const KisFilterOpSettings*>(_settings);
    Q_ASSERT(settings);
    KisPaintOp * op = new KisFilterOp(settings, _painter);
    return op;
}

KisPaintOpSettings *KisFilterOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP /*image*/, KisLayerSP /*layer*/)
{
    Q_UNUSED(inputDevice);
    return new KisFilterOpSettings(parent);
}

KisFilterOpSettings::KisFilterOpSettings(QWidget* parent) :
        QObject(parent),
        KisPaintOpSettings(parent),
        m_optionsWidget(new QWidget(parent)),
        m_uiOptions(new Ui_FilterOpOptions())
{
    m_uiOptions->setupUi(m_optionsWidget);

    // Check which filters support painting
    QList<KoID> l = KisFilterRegistry::instance()->listKeys();
    QList<KoID> l2;
    QList<KoID>::iterator it;
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->value((*it).id());
        if (f->supportsPainting()) {
            l2.push_back(*it);
        }
    }
    m_uiOptions->filtersList->setIDList( l2 );
    connect(m_uiOptions->filtersList, SIGNAL(activated(const KoID &)), SLOT(setCurrentFilter(const KoID &)));
}

KisFilterOpSettings::~KisFilterOpSettings()
{
    delete m_uiOptions;
}

void KisFilterOpSettings::setCurrentFilter(const KoID & id)
{
    m_currentFilter = KisFilterRegistry::instance()->get(id.id());
    m_currentFilterConfigWidget = m_currentFilter->createConfigurationWidget(0,0);
    m_uiOptions->popupButtonOptions->setPopupWidget(m_currentFilterConfigWidget);
}

KisFilterSP KisFilterOpSettings::filter() const
{
    return m_currentFilter;
}

KisFilterConfiguration* KisFilterOpSettings::filterConfig() const
{
    if(not m_currentFilterConfigWidget) return 0;
    return m_currentFilterConfigWidget->configuration();
}

KisFilterOp::KisFilterOp(const KisFilterOpSettings* settings, KisPainter * painter)
    : super(painter), m_settings(settings)
{
}

KisFilterOp::~KisFilterOp()
{
}

void KisFilterOp::paintAt(const KisPaintInformation& info)
{
    if (not painter())
    {
      kDebug() << "No painter !" << endl;
      return;
    }

    KisFilterSP filter = m_settings->filter();
    if (not filter)
    {
      kDebug() << "No filter !" << endl;
      return;
    }

    if ( not source() )
    {
      kDebug() << "No source !" << endl;
      return;
    }

    KisBrush * brush = painter()->brush();
    if (not brush) return;

    KoColorSpace * colorSpace = source()->colorSpace();

    QPointF hotSpot = brush->hotSpot(info);
    QPointF pt = info.pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    // Filters always work with a mask, never with an image; that
    // wouldn't be useful at all.
    KisQImagemaskSP mask = brush->mask(info, xFraction, yFraction);

    painter()->setPressure(info.pressure);

    qint32 maskWidth = mask->width();
    qint32 maskHeight = mask->height();

    // Create a temporary paint device
    KisPaintDeviceSP tmpDev = KisPaintDeviceSP(new KisPaintDevice(colorSpace, "filterop tmpdev"));
    Q_CHECK_PTR(tmpDev);

    // Copy the layer data onto the new paint device
#if 0
    KisPainter p( tmpDev );
    p.bitBlt( 0,  0,  colorSpace->compositeOp(COMPOSITE_COPY), source(), OPACITY_OPAQUE, x, y, maskWidth, maskHeight );

    // Filter the paint device
    filter->disableProgress();
    QRect r( 0, 0, maskWidth, maskHeight );
    filter->process( tmpDev, r, m_settings->filterConfig());
    filter->enableProgress();
#endif
    
    // Filter the paint device
    filter->disableProgress();
    filter->process(source(), QPoint(x,y), tmpDev, QPoint(0,0), QSize(maskWidth, maskHeight), m_settings->filterConfig());
    filter->enableProgress();
    
    // Apply the mask on the paint device (filter before mask because edge pixels may be important)

    KisHLineIterator hiter = tmpDev->createHLineIterator(0, 0, maskWidth);

    for (int y = 0; y < maskHeight; y++)
    {
        int x=0;
        while(! hiter.isDone())
        {
            quint8 alpha = mask->alphaAt( x++, y );
            colorSpace->setAlpha(hiter.rawData(), alpha, 1);

            ++hiter;
        }
        hiter.nextRow();

    }

    // Blit the paint device onto the layer
    QRect dabRect = QRect(0, 0, maskWidth, maskHeight);
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    if (painter()->bounds().isValid()) {
        dstRect &= painter()->bounds();
    }
    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    if (source()->hasSelection()) {
        painter()->bltSelection(dstRect.x(), dstRect.y(), painter()->compositeOp(), tmpDev,
                                source()->selection(), painter()->opacity(), sx, sy, sw, sh);
    }
    else {
        painter()->bitBlt(dstRect.x(), dstRect.y(), painter()->compositeOp(), tmpDev, painter()->opacity(), sx, sy, sw, sh);
    }

}

#include "kis_filterop.moc"
