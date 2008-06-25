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

#include <QRect>
#include <QColor>

#include <KoColor.h>

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

KisPaintOp * KisSumiPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image)
{
    Q_UNUSED( settings );
    Q_UNUSED( image );
    KisPaintOp * op = new KisSumiPaintOp(painter);
    Q_CHECK_PTR(op);
    return op;
}


KisSumiPaintOp::KisSumiPaintOp(KisPainter * painter)
    : KisPaintOp(painter)
{
    dbgKrita << "START OF KisSumiPaintOp" << endl;

}

KisSumiPaintOp::~KisSumiPaintOp()
{
}

void KisSumiPaintOp::paintAt(const KisPaintInformation& info)
{
    // KisPainter, see KisSumiPaintOp::createOp
    if (!painter()) return;

    // read, write pixel data
    KisPaintDeviceSP device = painter()->device();
    if (!device) return;
// boud
//    if (!painter()->device()) return;

    qint32 x = (qint32)info.pos().x();
    qint32 y = (qint32)info.pos().y();
    /*dbgKrita << "LUKAST: x" << x;
      dbgKrita << "LUKAST: y" << y;*/

    int r,g,b;
    r = x % 255;
    g = y % 255;
    b = (r - g);
    if (b<0) b = -b;
    b = b % 255;

    c.setRgb(r,g,b);
    //m_stroke->draw( dab );
    //setPixel (qint32 x, qint32 y, const QColor &c)

    // FASTER VERSION, but actually it is not faster..
    dab = cachedDab( );
    KisRandomAccessor accessor = dab->createRandomAccessor(x, y);

    accessor.moveTo(x,y);
    quint8 *pixel = accessor.rawData();
    quint32 val = c.rgba();
    memcpy(pixel, &val, sizeof(val) );

    // setPixel
    /*for (int i=0;i<20;i++)
      for (int j=0;j<20;j++)
      dab->setPixel(x+i,y+j, c.rgba() );*/

    QRect rc = dab->extent();
    painter()->bitBlt( rc.topLeft(), dab, rc );
}
