/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PAINTOP_REGISTRY_H_
#define KIS_PAINTOP_REGISTRY_H_

#include "kis_types.h"
#include "kis_generic_registry.h"
#include <koffice_export.h>

class QStringList;
class KisPaintop;
class KisPainter;

class KRITACORE_EXPORT KisPaintOpRegistry : public KisGenericRegistry<KisPaintOpFactorySP> {

public:
    virtual ~KisPaintOpRegistry();

    KisPaintOp * paintOp(const KisID& id, KisPainter * painter) const;
    KisPaintOp * paintOp(const QString& id, KisPainter * painter) const;

    // Whether we should show this paintop in the toolchest
    bool userVisible(const KisID & id) const;

    // Get the pixmap to show in the toolchest
    QPixmap getPixmap(const KisID & id) const;


public:
    static KisPaintOpRegistry* instance();

private:
    KisPaintOpRegistry();
    KisPaintOpRegistry(const KisPaintOpRegistry&);
    KisPaintOpRegistry operator=(const KisPaintOpRegistry&);

private:
    static KisPaintOpRegistry *m_singleton;
};

#endif // KIS_PAINTOP_REGISTRY_H_

