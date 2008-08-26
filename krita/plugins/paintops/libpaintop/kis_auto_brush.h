/*
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

#ifndef _KIS_AUTOBRUSH_RESOURCE_H_
#define _KIS_AUTOBRUSH_RESOURCE_H_

#include "krita_export.h"
#include "kis_brush.h"

class KisMaskGenerator;

class KRITAUI_EXPORT KisAutoBrush : public KisBrush
{
public:
    KisAutoBrush(KisMaskGenerator* img);
    virtual ~KisAutoBrush();
public:
    virtual void generateMask(KisPaintDeviceSP dst, KisBrush::ColoringInformation* src, double scaleX, double scaleY, double angle, const KisPaintInformation& info = KisPaintInformation(), double subPixelX = 0, double subPixelY = 0) const;
public:
    virtual bool load() { return false; }
    virtual void toXML(QDomDocument& , QDomElement&) const;
private:
    QImage createBrushPreview();
private:
    struct Private;
    Private* const d;
};
#endif // _KIS_AUTOBRUSH_RESOURCE_H_
