/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_CHALKPAINTOP_H_
#define KIS_CHALKPAINTOP_H_

#include <QColor>
#include <QMutex>

#include <klocale.h>
#include <kis_paintop.h>
#include <kis_types.h>

#include "brush.h"

class QPointF;
class KisPainter;
class KisChalkOpSettings;

class KisChalkPaintOpFactory : public KisPaintOpFactory {

public:
    KisChalkPaintOpFactory() {}
    virtual ~KisChalkPaintOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image);
    virtual QString id() const { return "chalkbrush"; }
    virtual QString name() const { return i18n("Chalk-e brush"); }
    virtual QString pixmap() { return "krita-chalk.png"; }
    virtual KisPaintOpSettingsSP settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image);
    virtual KisPaintOpSettingsSP settings(KisImageSP image);
};


class KisChalkPaintOp : public KisPaintOp {

public:

    KisChalkPaintOp(KisPainter * painter, KisImageSP image);
    virtual ~KisChalkPaintOp();

    void paintAt(const KisPaintInformation& info);

    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const
    {
        Q_UNUSED(xSpacing);
        Q_UNUSED(ySpacing);
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        // XXX: this is wrong!
        return 0.5;
    }



private:
    QColor c;
	QPointF m_previousPoint;
	KisImageSP m_image;
	bool newStrokeFlag;
	//Stroke stroke;
	KisPaintDeviceSP dab;
    QMutex m_mutex;
	Brush m_mybrush;
};

#endif // KIS_CHALKPAINTOP_H_
