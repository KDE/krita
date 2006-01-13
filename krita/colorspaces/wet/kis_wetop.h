/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_WETOP_H_
#define KIS_WETOP_H_

#include "kis_paintop.h"
#include "kis_types.h"
#include "kis_colorspace.h"
#include "wdgpressure.h"

class KisPoint;
class KisPainter;


class KisWetOpFactory : public KisPaintOpFactory  {
    WetPaintOptions* m_optWidget;
public:
    KisWetOpFactory() : m_optWidget(0) {}
    virtual ~KisWetOpFactory() {}

    virtual KisPaintOp * createOp(KisPainter * painter);
    virtual KisID id() { return KisID("wetbrush", i18n("Watercolor Brush")); }
    virtual bool userVisible(KisColorSpace* cs) { return cs -> id() == KisID("WET", ""); }
    virtual QWidget* createOptionWidget(QWidget* parent) {
        m_optWidget = new WetPaintOptions();
        return m_optWidget;
    }
    virtual QWidget* optionWidget() { return m_optWidget; }
};

class KisWetOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisWetOp(KisPainter * painter);
    virtual ~KisWetOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);

};

#endif // KIS_WETOP_H_
