/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <kdebug.h>
#include <qimage.h>

#include "kis_debug_areas.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"

KisAdjustmentLayer::KisAdjustmentLayer(KisImage *img, const QString &name, Q_UINT8 opacity) :
    KisLayer (img, name, opacity)
{
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
    : KisLayer(rhs)
{
}


KisAdjustmentLayerSP KisAdjustmentLayer::clone()
{
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
}


KisFilterConfiguration * KisAdjustmentLayer::filter()
{
}


void KisAdjustmentLayer::setFilter(KisFilterConfiguration * filterConfig)
{
}


KisSelectionSP KisAdjustmentLayer::selection()
{
}

void KisAdjustmentLayer::setSelection(KisSelectionSP selection)
{
}



Q_INT32 KisAdjustmentLayer::x() const
{
}

void KisAdjustmentLayer::setX(Q_INT32)
{
}

Q_INT32 KisAdjustmentLayer::y() const
{
}

void KisAdjustmentLayer::setY(Q_INT32)
{
}

QRect KisAdjustmentLayer::extent() const
{
}
    
QRect KisAdjustmentLayer::exactBounds() const
{
}

#include "kis_adjustment_layer.moc"
