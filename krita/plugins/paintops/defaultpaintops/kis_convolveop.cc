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
#include <QRect>

#include <kdebug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_convolveop.h"


KisPaintOp * KisConvolveOpFactory::createOp(const KisPaintOpSettings */*settings*/, KisPainter * painter)
{ 
    KisPaintOp * op = new KisConvolveOp(painter); 
    Q_CHECK_PTR(op);
    return op; 
}


KisConvolveOp::KisConvolveOp(KisPainter * painter)
    : super(painter) 
{
}

KisConvolveOp::~KisConvolveOp() 
{
}

void KisConvolveOp::paintAt(const KisPoint &/*pos*/, const KisPaintInformation& /*info*/)
{
    // XXX: use convolve painter here.
    
}
