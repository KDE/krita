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

#include <QObject>

#include "kis_types.h"
#include "KoGenericRegistry.h"
#include <krita_export.h>

class QWidget;
class QStringList;

class KisPaintOp;
class KisPaintOpSettings;
class KisPainter;
class KoColorSpace;
class KisInputDevice;

class KRITAIMAGE_EXPORT KisPaintOpRegistry : public QObject, public KisGenericRegistry<KisPaintOpFactorySP>
{

    Q_OBJECT

public:
    virtual ~KisPaintOpRegistry();

    /**
     * Return a newly created paintop
     */
    KisPaintOp * paintOp(const KoID& id, const KisPaintOpSettings * settings, KisPainter * painter) const;

    /**
     * Return a newly created paintopd
     */
    KisPaintOp * paintOp(const QString& id, const KisPaintOpSettings * settings, KisPainter * painter) const;

    /**
     * Create and return an (abstracted) configuration widget
     * for using the specified paintop with the specified input device,
     * with the specified parent as widget parent. Returns 0 if there
     * are no settings available for the given device.
     */
    KisPaintOpSettings * settings(const KoID& id, QWidget * parent, const KisInputDevice& inputDevice) const;

    // Whether we should show this paintop in the toolchest
    bool userVisible(const KoID & id, KoColorSpace* cs) const;

    // Get the name of the icon to show in the toolchest
    QString pixmap(const KoID & id) const;


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

