/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_PAINT_DEVICE_VISITOR_H_
#define KIS_PAINT_DEVICE_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device_impl.h"
#include "kis_layer.h"
#include "kis_selection.h"

class KisPaintDeviceImplVisitor {
public:
    KisPaintDeviceImplVisitor();
    virtual ~KisPaintDeviceImplVisitor();

public:
    virtual bool visit(KisPainter& gc, vKisPaintDeviceImplSP& devs) = 0;
    virtual bool visit(KisPainter& gc, KisPaintDeviceImplSP dev) = 0;
    virtual bool visit(KisPainter& gc, vKisLayerSP& layers) = 0;
    virtual bool visit(KisPainter& gc, KisLayerSP layer) = 0;
    virtual bool visit(KisPainter& gc, KisSelectionSP selection) = 0;

public:
    bool operator()(KisPainter& gc, vKisPaintDeviceImplSP& devs);
    bool operator()(KisPainter& gc, KisPaintDeviceImplSP dev);
    bool operator()(KisPainter& gc, vKisLayerSP& layers);
    bool operator()(KisPainter& gc, KisLayerSP layer);
    bool operator()(KisPainter& gc, KisSelectionSP selection);
};

inline
KisPaintDeviceImplVisitor::KisPaintDeviceImplVisitor()
{
}

inline
KisPaintDeviceImplVisitor::~KisPaintDeviceImplVisitor()
{
}

inline
bool KisPaintDeviceImplVisitor::operator()(KisPainter& gc, vKisPaintDeviceImplSP& devs)
{
    return visit(gc, devs);
}

inline
bool KisPaintDeviceImplVisitor::operator()(KisPainter& gc, KisPaintDeviceImplSP dev)
{
    return visit(gc, dev);
}

inline
bool KisPaintDeviceImplVisitor::operator()(KisPainter& gc, vKisLayerSP& layers)
{
    return visit(gc, layers);
}

inline
bool KisPaintDeviceImplVisitor::operator()(KisPainter& gc, KisLayerSP layer)
{
    return visit(gc, layer);
}

inline
bool KisPaintDeviceImplVisitor::operator()(KisPainter& gc, KisSelectionSP selection)
{
    return visit(gc, selection);
}


#endif // KIS_PAINT_DEVICE_VISITOR_H_

