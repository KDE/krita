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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KIS_PAINT_DEVICE_VISITOR_H_
#define KIS_PAINT_DEVICE_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_floatingselection.h"

class KisPaintDeviceVisitor {
public:
	KisPaintDeviceVisitor();
	virtual ~KisPaintDeviceVisitor();

public:
	virtual bool visit(KisPainter& gc, vKisPaintDeviceSP& devs) = 0;
	virtual bool visit(KisPainter& gc, KisPaintDeviceSP dev) = 0;
	virtual bool visit(KisPainter& gc, vKisLayerSP& layers) = 0;
	virtual bool visit(KisPainter& gc, KisLayerSP layer) = 0;
	virtual bool visit(KisPainter& gc, KisFloatingSelectionSP selection) = 0;

public:
	bool operator()(KisPainter& gc, vKisPaintDeviceSP& devs);
	bool operator()(KisPainter& gc, KisPaintDeviceSP dev);
	bool operator()(KisPainter& gc, vKisLayerSP& layers);
	bool operator()(KisPainter& gc, KisLayerSP layer);
	bool operator()(KisPainter& gc, KisFloatingSelectionSP selection);
};

inline
KisPaintDeviceVisitor::KisPaintDeviceVisitor()
{
}

inline
KisPaintDeviceVisitor::~KisPaintDeviceVisitor()
{
}

inline
bool KisPaintDeviceVisitor::operator()(KisPainter& gc, vKisPaintDeviceSP& devs)
{
	return visit(gc, devs);
}

inline
bool KisPaintDeviceVisitor::operator()(KisPainter& gc, KisPaintDeviceSP dev)
{
	return visit(gc, dev);
}

inline
bool KisPaintDeviceVisitor::operator()(KisPainter& gc, vKisLayerSP& layers)
{
	return visit(gc, layers);
}

inline
bool KisPaintDeviceVisitor::operator()(KisPainter& gc, KisLayerSP layer)
{
	return visit(gc, layer);
}

inline
bool KisPaintDeviceVisitor::operator()(KisPainter& gc, KisFloatingSelectionSP selection)
{
	return visit(gc, selection);
}

#endif // KIS_PAINT_DEVICE_VISITOR_H_

