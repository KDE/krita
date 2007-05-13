/*
 * This file is part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_SCRIPT_FILTER_H_
#define _KIS_SCRIPT_FILTER_H_

#include <QObject>
#include "kis_filter.h"

#include "kritacore/krs_paint_device.h"

namespace Kross {
    class Action;
}

class KisScriptFilter : public KisFilter {
    Q_OBJECT
    public:
        KisScriptFilter(Kross::Action* action);
        virtual void process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config);
    signals:
        void scriptProcess( Scripting::PaintDevice* src, const QPoint& srcTopLeft, Scripting::ConstPaintDevice* dst, const QPoint& dstTopLeft, const QSize& size, QObject* config );
};


#endif
