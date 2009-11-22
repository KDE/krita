/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *           (C) Dmitry Kazakov <dimula73@gmail.com>, 2009
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_merge_visitor.h"

bool KisMergeVisitor::compositeWithProjection(KisLayer *layer, const QRect &rect)
{

    Q_ASSERT(m_projection);
    if (!layer->visible()) return true;

    KisPaintDeviceSP device = layer->projection();
    if (!device) return true;

    Q_ASSERT(layer->compositeOp());
    if (!layer->compositeOp()) return false;

    QRect needRect = rect & device->extent();

    KisPainter gc(m_projection);
    gc.setChannelFlags(layer->channelFlags());
    gc.setCompositeOp(layer->compositeOp());
    gc.setOpacity(layer->opacity());
    gc.bitBlt(needRect.topLeft(), device, needRect);

    return true;
}


