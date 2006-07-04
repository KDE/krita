/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

/* Initial Commit using skel from kis_fill_painter. Emanuele Tamponi */

#ifndef KIS_CURVES_PAINTER_H_
#define KIS_CURVES_PAINTER_H_

#include "kis_meta_registry.h"
#include "kis_color.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_painter.h"
#include "kis_types.h"
#include <koffice_export.h>

class KisCurvesPainter : public KisPainter
{

    typedef KisPainter super;

public:

    KisCurvesPainter();

    KisCurvesPainter(KisPaintDeviceSP device);

};

#endif //KIS_CURVES_PAINTER_H_
