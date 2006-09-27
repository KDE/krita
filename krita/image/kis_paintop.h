/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_PAINTOP_H_
#define KIS_PAINTOP_H_

#include <QString>

#include <ksharedptr.h>
#include <klocale.h>

#include "kis_global.h"
#include "kis_types.h"
#include "KoID.h"
#include "kis_vec.h"
#include "KoColorSpace.h"

#include <krita_export.h>

class KisPoint;
class KisAlphaMask;
class KisPainter;
class KoColorSpace;
class KisInputDevice;
class QWidget;

/**
 * This class keeps information that can be used in the painting process, for example by
 * brushes.
 **/
class KRITAIMAGE_EXPORT KisPaintInformation {
public:
    KisPaintInformation(double pressure = PRESSURE_DEFAULT,
                        double xTilt = 0.0, double yTilt = 0.0,
                        KisVector2D movement = KisVector2D())
        : pressure(pressure), xTilt(xTilt), yTilt(yTilt), movement(movement) {}
    double pressure;
    double xTilt;
    double yTilt;
    KisVector2D movement;
};

class KRITAIMAGE_EXPORT KisPaintOp : public KShared
{

public:

    KisPaintOp(KisPainter * painter);
    virtual ~KisPaintOp();

    virtual void paintAt(const KisPoint &pos, const KisPaintInformation& info) = 0;
    void setSource(KisPaintDeviceSP p);

    /**
     * Whether this paintop wants to deposit paint even when not moving, i.e. the
     * tool needs to activate its timer.
     */
    virtual bool incremental() { return false; }


protected:

    virtual KisPaintDeviceSP computeDab(KisAlphaMaskSP mask);
    virtual KisPaintDeviceSP computeDab(KisAlphaMaskSP mask, KoColorSpace *cs);


    /**
     * Split the coordinate into whole + fraction, where fraction is always >= 0.
     */
    virtual void splitCoordinate(double coordinate, qint32 *whole, double *fraction);

    KisPainter * m_painter;
    KisPaintDeviceSP m_source; // use this layer as source layer for the operation
private:
    KisPaintDeviceSP m_dab;
};

class KisPaintOpSettings {
public:
    KisPaintOpSettings(QWidget *parent) { Q_UNUSED(parent); }
    virtual ~KisPaintOpSettings() {}

    virtual QWidget *widget() const { return 0; }
};

/**
 * The paintop factory is responsible for creating paintops of the specified class.
 * If there is an optionWidget, the derived paintop itself must support settings,
 * and it's up to the factory to do that.
 */
class KRITAIMAGE_EXPORT KisPaintOpFactory  : public KShared
{

public:
    KisPaintOpFactory() {}
    virtual ~KisPaintOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter) = 0;
    virtual KoID id() { return KoID("abstractpaintop", i18n("Abstract PaintOp")); }

    /**
     * The filename of the pixmap we can use to represent this paintop in the ui.
     */
    virtual QString pixmap() { return ""; }

    /**
     * Whether this paintop is internal to a certain tool or can be used
     * in various tools. If false, it won't show up in the toolchest.
     * The KoColorSpace argument can be used when certain paintops only support a specific cs
     */
    virtual bool userVisible(KoColorSpace * cs = 0) { return cs->id() != KoID("WET", ""); }

    /**
     * Create and return an (abstracted) widget with options for this paintop when used with the
     * specified input device. Return 0 if there are no settings available for the given
     * device.
     */
    virtual KisPaintOpSettings* settings(QWidget* parent, const KisInputDevice& inputDevice);

};
#endif // KIS_PAINTOP_H_
