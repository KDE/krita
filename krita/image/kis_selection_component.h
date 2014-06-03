/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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
#ifndef KIS_SELECTION_COMPONENT_H
#define KIS_SELECTION_COMPONENT_H

#include <krita_export.h>

#include <QTransform>

class QRect;
class KUndo2Command;
class KisSelection;
class KisPixelSelection;

#include "kis_types.h"

class KRITAIMAGE_EXPORT KisSelectionComponent
{
public:
    KisSelectionComponent() {}
    virtual ~KisSelectionComponent() {}

    virtual KisSelectionComponent* clone(KisSelection* selection) = 0;

    virtual void renderToProjection(KisPaintDeviceSP projection) = 0;
    virtual void renderToProjection(KisPaintDeviceSP projection, const QRect& r) = 0;

    virtual void moveX(qint32 x) { Q_UNUSED(x); }
    virtual void moveY(qint32 y) { Q_UNUSED(y); }

    virtual KUndo2Command* transform(const QTransform &transform) {
        Q_UNUSED(transform);
        return 0;
    }

    virtual bool isEmpty() const = 0;
    virtual QPainterPath outlineCache() const = 0;
    virtual bool outlineCacheValid() const = 0;
    virtual void recalculateOutlineCache() = 0;

    virtual KUndo2Command* resetToEmpty() { return 0; }
};

#endif
