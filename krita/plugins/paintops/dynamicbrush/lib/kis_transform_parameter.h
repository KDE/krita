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

#ifndef _KIS_TRANSFORM_PARAMETER_H_
#define _KIS_TRANSFORM_PARAMETER_H_

#include <kis_paintop.h>
#include <KoID.h>

class KisTransformParameter {
    public:
        virtual ~KisTransformParameter() { }
        virtual double parameter(const KisPaintInformation& info) = 0;
        static KisTransformParameter* id2TransformParameter(const KoID&);
        static QList<KoID> transformParameterIds();
};

class KisTransformParameterPressure : public KisTransformParameter {
    public:
    virtual ~KisTransformParameterPressure() { }
        virtual double parameter(const KisPaintInformation& info)
        { return info.pressure; }
};

class KisTransformParameterXTilt : public KisTransformParameter {
    public:
    virtual ~KisTransformParameterXTilt() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 - info.xTilt; }
};

class KisTransformParameterYTilt : public KisTransformParameter {
    public:
    virtual ~KisTransformParameterYTilt() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 - info.yTilt; }
};

#endif
