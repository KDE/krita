/*
 *  Copyright (c) 2006-2007,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef _KIS_DYNAMIC_SENSORS_H_
#define _KIS_DYNAMIC_SENSORS_H_

#include "../kis_dynamic_sensor.h"

#include <kis_paintop.h>
#include <KoID.h>

#include "kis_paint_information.h"

class KisDynamicSensorFuzzy : public KisDynamicSensor
{
public:
    KisDynamicSensorFuzzy();
    virtual ~KisDynamicSensorFuzzy() { }
    virtual qreal value(const KisPaintInformation&) {
        return rand() / (qreal)RAND_MAX;
    }
};

class KisDynamicSensorSpeed : public KisDynamicSensor
{
public:
    KisDynamicSensorSpeed();
    virtual ~KisDynamicSensorSpeed() { }
    virtual qreal value(const KisPaintInformation& info);
    void reset() {
        m_lastTime = 0;
        m_lastMove = 0.0;
        m_speed = 0.0;
    }
private:
    double m_speed;
    double m_lastMove;
    int m_lastTime;
};

class KisDynamicSensorDrawingAngle : public KisDynamicSensor
{
public:
    KisDynamicSensorDrawingAngle();
    virtual ~KisDynamicSensorDrawingAngle() { }
    virtual qreal value(const KisPaintInformation& info);
};

class KisDynamicSensorRotation : public KisDynamicSensor
{
public:
    KisDynamicSensorRotation();
    virtual ~KisDynamicSensorRotation() { }
    virtual qreal value(const KisPaintInformation& info) {
        return info.rotation() / 28 + 0.5; // it appears that rotation is between -14 and +14
    }
};

class KisDynamicSensorPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorPressure();
    virtual ~KisDynamicSensorPressure() { }
    virtual qreal value(const KisPaintInformation& info) {
        return info.pressure();
    }
};

class KisDynamicSensorXTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorXTilt();
    virtual ~KisDynamicSensorXTilt() { }
    virtual qreal value(const KisPaintInformation& info) {
        return 1.0 - fabs(info.xTilt()) / 60.0;
    }
};

class KisDynamicSensorYTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorYTilt();
    virtual ~KisDynamicSensorYTilt() { }
    virtual qreal value(const KisPaintInformation& info) {
        return 1.0 - fabs(info.yTilt()) / 60.0;
    }
};

class KisDynamicSensorAscension : public KisDynamicSensor
{
public:
    KisDynamicSensorAscension();
    virtual ~KisDynamicSensorAscension() {}
    virtual qreal value(const KisPaintInformation& info){
        qreal xTilt = info.xTilt();
        qreal yTilt = info.yTilt();
        // radians -PI, PI
        qreal ascension = atan2(-xTilt, yTilt);
        // map to 0.0..1.0
        ascension = ascension / (2 * M_PI) + 0.5;
        return ascension;
    }
};

class KisDynamicSensorDeclination : public KisDynamicSensor
{
public:
    KisDynamicSensorDeclination();
    virtual ~KisDynamicSensorDeclination() {}
    virtual qreal value(const KisPaintInformation& info){
        qreal xTilt = qBound(-1.0, info.xTilt() / 60.0 , 1.0);
        qreal yTilt = qBound(-1.0, info.yTilt() / 60.0 , 1.0);

        qreal e;
        if (fabs(xTilt) > fabs(yTilt)) {
            e = sqrt(1.0 + yTilt*yTilt);
        } else {
            e = sqrt(1.0 + xTilt*xTilt);
        }
      
        qreal cosAlpha = sqrt(xTilt*xTilt + yTilt*yTilt)/e;
        // in radians in [0, 0.5 * PI] .. mapping to 0.0..1.0
        qreal declination = acos (cosAlpha) / (M_PI * 0.5);
        return declination;
    }
};

class KisDynamicSensorPerspective : public KisDynamicSensor
{
public:
    KisDynamicSensorPerspective();
    virtual ~KisDynamicSensorPerspective() { }
    virtual qreal value(const KisPaintInformation& info) {
        return info.perspective();
    }
};

#endif
