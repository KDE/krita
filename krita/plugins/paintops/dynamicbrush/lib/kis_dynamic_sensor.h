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

#include "dynamicbrush_export.h"

#include <kis_paintop.h>
#include <KoID.h>

class QListString;

class DYNAMIC_BRUSH_EXPORT KisDynamicSensor {
    public:
        KisDynamicSensor(const KoID& id);
        virtual ~KisDynamicSensor() { }
        virtual double parameter(const KisPaintInformation& info) = 0;
        static KisDynamicSensor* id2Sensor(const KoID&);
        static KisDynamicSensor* id2Sensor(const QString& s) { return id2Sensor(KoID(s)); }
        static QList<KoID> sensorsIds();
        inline const KoID& id() { return m_id; }
    private:
        const KoID& m_id;
};

class KisDynamicSensorSpeed : public KisDynamicSensor {
    public:
        KisDynamicSensorSpeed();
        virtual ~KisDynamicSensorSpeed() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 + info.movement.length() * 0.1; }
};

class KisDynamicSensorPressure : public KisDynamicSensor {
    public:
        KisDynamicSensorPressure();
        virtual ~KisDynamicSensorPressure() { }
        virtual double parameter(const KisPaintInformation& info)
        { return info.pressure; }
};

class KisDynamicSensorXTilt : public KisDynamicSensor {
    public:
        KisDynamicSensorXTilt();
        virtual ~KisDynamicSensorXTilt() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 - info.xTilt; }
};

class KisDynamicSensorYTilt : public KisDynamicSensor {
    public:
        KisDynamicSensorYTilt();
        virtual ~KisDynamicSensorYTilt() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 - info.yTilt; }
};

#endif
