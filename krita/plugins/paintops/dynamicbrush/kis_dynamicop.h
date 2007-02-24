/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DYNAMICOP_H_
#define KIS_DYNAMICOP_H_

#include "kis_paintop.h"

class QWidget;
class QCheckBox;
class QLabel;
class QPointF;
class KisPainter;

class KisDynamicBrush;

class KisDynamicOpFactory : public KisPaintOpFactory  {
public:
    KisDynamicOpFactory() {}
    virtual ~KisDynamicOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KoID id() { return KoID("dynamicbrush", i18n("Dynamic Brush")); }
    virtual QString pixmap() { return "dynamicbrush.png"; }
};

class KisDynamicOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisDynamicOp(KisPainter * painter);
    virtual ~KisDynamicOp();

    void paintAt(const QPointF &pos, const KisPaintInformation& info);

private:
    KisDynamicBrush* m_brush;
};

#endif // KIS_DYNAMICOP_H_
