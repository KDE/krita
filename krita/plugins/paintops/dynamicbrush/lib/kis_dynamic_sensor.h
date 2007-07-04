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

#include <QObject>

#include <KoID.h>

#include <klocale.h>

class QWidget;
class KisPaintInformation;

const KoID FuzzyId("fuzzy", i18n("Fuzzy"));
const KoID SpeedId("speed", i18n("Speed"));
const KoID TimeId("time", i18n("Time"));
const KoID DrawingAngleId("drawingangle", i18n("Drawing angle"));
const KoID PressureId("pressure", i18n("Pressure"));
const KoID XTiltId ("xtilt", i18n("X-Tilt"));
const KoID YTiltId ("ytilt", i18n("Y-Tilt"));

/**
 * Sensor are used to extract from KisPaintInformation a single
 * double value which can be used to control 
 */
class DYNAMIC_BRUSH_EXPORT KisDynamicSensor : public QObject {
    public:
        enum ParameterSign {
            NegativeParameter = -1,
            UnSignedParameter = 0,
            PositiveParameter = 1
        };
    protected:
        KisDynamicSensor(const KoID& id);
    public:
        virtual ~KisDynamicSensor() { }
        /**
         * @return the value of this sensor for the given KisPaintInformation
         */
        virtual double parameter(const KisPaintInformation& info) = 0;
        virtual QWidget* createConfigurationWidget(QWidget* parent);
        /**
         * Creates a sensor from its identifiant.
         */
        static KisDynamicSensor* id2Sensor(const KoID&);
        static KisDynamicSensor* id2Sensor(const QString& s) { return id2Sensor(KoID(s)); }
        /**
         * @return the list of sensors
         */
        static QList<KoID> sensorsIds();
        /**
         * @return the identifiant of this sensor
         */
        inline const KoID& id() { return m_id; }
    private:
        const KoID& m_id;
};

#endif
