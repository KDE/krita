/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_sumipaintop.h"
#include <cmath>

#include <QRect>
#include <QColor>
#include <QMutexLocker>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_random_accessor.h"

#include "kis_datamanager.h"

#include "lines.h"
#include "brush.h"

KisPaintOp * KisSumiPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image)
{
    Q_UNUSED( settings );
    KisPaintOp * op = new KisSumiPaintOp(painter, image);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisSumiPaintOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image)
{
    Q_UNUSED( inputDevice );
    Q_UNUSED( image );
    return new KisSumiPaintOpSettings( parent );
}

KisPaintOpSettingsSP KisSumiPaintOpFactory::settings(KisImageSP image)
{
    Q_UNUSED( image );
    return new KisSumiPaintOpSettings( 0 );
}

KisSumiPaintOpSettings::KisSumiPaintOpSettings(QWidget * parent )
    : KisPaintOpSettings()
{
    m_optionsWidget = new KisPopupButton( parent );
    m_optionsWidget->setText( "..." );
    m_optionsWidget->setToolTip( i18n( "Options for the sumi-e brush" ) );

    m_popupWidget = new QWidget( parent );
    m_options = new Ui::WdgSumieOptions( );
    m_options->setupUi( m_popupWidget );

    m_optionsWidget->setPopupWidget(m_popupWidget);

}

KisPaintOpSettingsSP KisSumiPaintOpSettings::clone() const
{
    KisSumiPaintOpSettings* s = new KisSumiPaintOpSettings( 0 );
    return s;

}

void KisSumiPaintOpSettings::fromXML(const QDomElement&)
{
    // XXX: save to xml. See for instance the color adjustment filters
}

void KisSumiPaintOpSettings::toXML(QDomDocument&, QDomElement&) const
{
    // XXX: load from xml. See for instance the color adjustment filters
}



KisSumiPaintOp::KisSumiPaintOp(KisPainter * painter, KisImageSP image)
    : KisPaintOp(painter)
{
    dbgKrita << "START OF KisSumiPaintOp" << endl;
    newStrokeFlag = true;
    m_image = image;
}

KisSumiPaintOp::~KisSumiPaintOp()
{
    dbgKrita << "END OF KisSumiPaintOp" << endl;
}

void KisSumiPaintOp::paintAt(const KisPaintInformation& info)
{
    QMutexLocker locker(&m_mutex);
    if (!painter()) return;
    // read, write pixel data
    KisPaintDeviceSP device = painter()->device();
    if (!device) return;

    if ( newStrokeFlag ) {
        newStrokeFlag = false;
    } else
    {
	dab = cachedDab( );
	dab->clear();

	float x1,y1;

	x1 = info.pos().x();
	y1 = info.pos().y();

	m_mybrush.paint(dab, x1, y1, painter()->paintColor() );

	QRect rc = dab->extent();
	painter()->bitBlt( rc.topLeft(), dab, rc );
    }

    //painter()->bltSelection(x, y, painter()->compositeOp(), dab, painter()->opacity(), x, y, 1, 1);
}


/*double KisSumiPaintOp::paintLine(const KisPaintInformation &pi1,
  const KisPaintInformation &pi2,
  double savedDist )
  {
  Q_UNUSED(savedDist);

  if (!painter()) return -1;
  // read, write pixel data
  KisPaintDeviceSP device = painter()->device();
  if (!device) return -1;

  dab = cachedDab( );
  dab->clear();

  float x0,y0,x1,y1;
  x0 = pi1.pos().x();
  y0 = pi1.pos().y();

  x1 = pi2.pos().x();
  y1 = pi2.pos().y();

  Lines lines;
  //lines.drawGSLine(dab,(int)x0, (int)y0, (int)x1, (int)y1,10,5, painter()->paintColor() );
  // Feel free to uncomment any line
  //lines.drawDDALine(dab, (int)x0, (int)y0, (int)x1, (int)y1,painter()->paintColor() );
  lines.drawWuLine(dab,(int)x0, (int)y0, (int)x1, (int)y1, 1 ,painter()->paintColor() );

  QRect rc = dab->extent();
  painter()->bitBlt( rc.topLeft(), dab, rc );

  return 0;
  }*/
