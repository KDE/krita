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
#include <QMetaType>
#include <QRegion>

#include <ksharedptr.h>
#include <klocale.h>

#include "KoID.h"
#include "KoColorSpace.h"

#include "kis_shared.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_paint_information.h"

#include <krita_export.h>

class QWidget;

class QPointF;
class KoColorSpace;
class KoInputDevice;

class KisQImagemask;
class KisPainter;

/**
 * KisPaintOp are use by tools to draw on a paint device.
 */
class KRITAIMAGE_EXPORT KisPaintOp : public KisShared
{

public:

    KisPaintOp(KisPainter * painter);
    virtual ~KisPaintOp();

    /**
     * Paint at the subpixel point pos using the specified paint
     * information..
     */
    virtual void paintAt(const QPointF &pos, const KisPaintInformation& info) = 0;
    void setSource(KisPaintDeviceSP p);

    /**
     * Whether this paintop wants to deposit paint even when not moving, i.e. the
     * tool needs to activate its timer.
     */
    virtual bool incremental() { return false; }


protected:

    virtual KisPaintDeviceSP computeDab(KisQImagemaskSP mask);
    virtual KisPaintDeviceSP computeDab(KisQImagemaskSP mask, KoColorSpace *cs);


    /**
     * Split the coordinate into whole + fraction, where fraction is always >= 0.
     */
    virtual void splitCoordinate(double coordinate, qint32 *whole, double *fraction);

    KisPainter * m_painter;
    KisPaintDeviceSP m_source; // use this layer as source layer for the operation
private:
    KisPaintDeviceSP m_dab;
};

/**
 * This class is used to cache the settings (and the associated widget) for a paintop
 * between two creation. There is one KisPaintOpSettings per input device (mouse, tablet, etc...).
 */
class KisPaintOpSettings {

public:
    KisPaintOpSettings() {}
    KisPaintOpSettings(QWidget *parent) { Q_UNUSED(parent); }
    virtual ~KisPaintOpSettings() {}

    /**
     * @return a pointer to the widget displaying the settings
     */
    virtual QWidget *widget() const { return 0; }
};

/**
 * The paintop factory is responsible for creating paintops of the specified class.
 * If there is an optionWidget, the derived paintop itself must support settings,
 * and it's up to the factory to do that.
 */
class KRITAIMAGE_EXPORT KisPaintOpFactory  : public KisShared
{

public:
    KisPaintOpFactory() {}
    virtual ~KisPaintOpFactory() {}

    /**
     * Create a KisPaintOp with the given settings and painter.
     * @param settings the settings associated with the input device
     * @param painter the painter used to draw
     */
    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter) = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;

    /**
     * The filename of the pixmap we can use to represent this paintop in the ui.
     */
    virtual QString pixmap() { return ""; }

    /**
     * Whether this paintop is internal to a certain tool or can be used
     * in various tools. If false, it won't show up in the toolchest.
     * The KoColorSpace argument can be used when certain paintops only support a specific cs
     */
    virtual bool userVisible(KoColorSpace * cs = 0) { return cs && cs->id() != "WET"; }

    /**
     * Create and return an (abstracted) widget with options for this paintop when used with the
     * specified input device. Return 0 if there are no settings available for the given
     * device.
     */
    virtual KisPaintOpSettings* settings(QWidget* parent, const KoInputDevice& inputDevice);

};
#endif // KIS_PAINTOP_H_
