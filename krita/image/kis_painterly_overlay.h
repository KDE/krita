/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
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
#ifndef KIS_PAINTERLY_OVERLAY
#define KIS_PAINTERLY_OVERLAY

#include <kicon.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_painterly_overlay_colorspace.h"
#include "kis_node.h"

#include <krita_export.h>

const QString KIS_PAINTERLY_OVERLAY_ID = "KisPainterlyOverlay";
/**
 * KisPainterlyOverlay is a special paintdevice that uses the
 * KisPainterlyOverlayColorSpace to describe the data pertinent to
 * canvas and painterly medium.
 *
 * XXX: Consider whether we should separate canvas properties and
 * medium properties.
 *
 * See the unittest for example code.
 */
class KRITAIMAGE_EXPORT KisPainterlyOverlay : public KisPaintDevice {

Q_OBJECT

public:

    KisPainterlyOverlay();
    virtual ~KisPainterlyOverlay();

    QIcon icon() const
        {
            return KIcon(""); // XXX: Find nice icon for the subclasses.
        }

    virtual QString nodeType()
        {
            return KIS_PAINTERLY_OVERLAY_ID;
        }

    virtual bool canHaveChildren()
        {
            return false;
        }
private:

    class Private;
    Private * const m_d;

};

#endif
