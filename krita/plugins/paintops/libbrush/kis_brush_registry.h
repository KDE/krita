/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_BRUSH_REGISTRY_H_
#define KIS_BRUSH_REGISTRY_H_

#include <QObject>

#include "kis_types.h"
#include "KoGenericRegistry.h"

#include <krita_export.h>

#include "kis_brush.h"
#include "kis_brush_factory.h"

class QString;
class QDomElement;

class BRUSH_EXPORT KisBrushRegistry : public QObject, public KoGenericRegistry<KisBrushFactory*>
{

    Q_OBJECT

public:
    virtual ~KisBrushRegistry();

    static KisBrushRegistry* instance();

    KisBrushSP getOrCreateBrush(const QDomElement& element);

private:
    KisBrushRegistry();
    KisBrushRegistry(const KisBrushRegistry&);
    KisBrushRegistry operator=(const KisBrushRegistry&);
};

#endif // KIS_GENERATOR_REGISTRY_H_
