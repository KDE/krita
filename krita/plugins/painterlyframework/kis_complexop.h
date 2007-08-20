/*
 *  Copyright (c) 2007 Emanuele Tamponi (emanuele@valinor.it)
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

#ifndef KIS_COMPLEXOP_H_
#define KIS_COMPLEXOP_H_

#include <klocale.h>
#include "kis_paintop.h"
#include "kis_painterly_overlay_colorspace.h"

class QPointF;
class KisPainter;

class KisComplexOpFactory : public KisPaintOpFactory  {

public:
    KisComplexOpFactory() {}
    virtual ~KisComplexOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter *painter, KisImageSP image);
    virtual QString id() const { return "paintcomplex"; }
    virtual QString name() const { return i18n("Complex Brush"); }
    virtual QString pixmap() { return ""; }

};


class KisComplexOp : public KisPaintOp {

    typedef KisPaintOp super;

	public:

		KisComplexOp(KisPainter *painter);
		virtual ~KisComplexOp();

		void paintAt(const KisPaintInformation& info);
		bool painterly() const {return true;}

	private:

		void mixChannels(QVector<float> &mixed, const QVector<float> &val1, float vol1, const QVector<float> &val2, float vol2);
		void mixProperty(PropertyCell *mixed, const PropertyCell *cell1, float vol1, const PropertyCell *cell2, float vol2);
		void computePaintTransferAmount(PropertyCell *brush,
										PropertyCell *canvas,
										float pressure, float velocity,
										float &volume_bc, float &volume_cb);

};

#endif // KIS_COMPLEXOP_H_
