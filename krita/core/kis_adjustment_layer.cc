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

KisAdjustmentLayer::KisAdjustmentLayer(KisImageSP img, const QString &name) :
    KisLayer (img, name, OPACITY_OPAQUE)
{
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
    : KisLayer(rhs)
{
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
}


KisLayerSP KisAdjustmentLayer::clone() const
{
    return new KisAdjustmentLayer(*this);
}


KisFilterConfiguration * KisAdjustmentLayer::filter()
{
    return 0;
}


void KisAdjustmentLayer::setFilter(KisFilterConfiguration * filterConfig)
{
}


KisSelectionSP KisAdjustmentLayer::selection()
{
    return 0;
}

void KisAdjustmentLayer::setSelection(KisSelectionSP selection)
{
}



Q_INT32 KisAdjustmentLayer::x() const
{
    return 0;
}

void KisAdjustmentLayer::setX(Q_INT32)
{
}

Q_INT32 KisAdjustmentLayer::y() const
{
    return 0;
}

void KisAdjustmentLayer::setY(Q_INT32)
{
}

QRect KisAdjustmentLayer::extent() const
{
    return QRect();
}
    
QRect KisAdjustmentLayer::exactBounds() const
{
    return QRect();
}

bool KisAdjustmentLayer::accept(KisLayerVisitor & v)
{
    return v.visit( this );
}

#include "kis_adjustment_layer.moc"
