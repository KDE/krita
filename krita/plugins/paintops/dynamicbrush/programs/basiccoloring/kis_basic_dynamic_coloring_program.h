/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_BASIC_DYNAMIC_COLORING_PROGRAM_H_
#define _KIS_BASIC_DYNAMIC_COLORING_PROGRAM_H_

#include "kis_dynamic_coloring_program.h"

class KisDynamicSensor;

class KisBasicDynamicColoringProgram : public KisDynamicColoringProgram {
    Q_OBJECT
    public:
        KisBasicDynamicColoringProgram(const QString& name);
        virtual ~KisBasicDynamicColoringProgram();
        virtual void apply(KisDynamicColoring* coloring, const KisPaintInformation& adjustedInfo) const;
        virtual QWidget* createEditor(QWidget* parent);
        virtual void fromXML(const QDomElement&);
        virtual void toXML(QDomDocument&, QDomElement&) const;
    public:
        bool isMixerEnabled() const;
        int mixerJitter() const;
        KisDynamicSensor* mixerSensor() const;
        bool isHueEnabled() const;
        int hueJitter() const;
        KisDynamicSensor* hueSensor() const;
        bool isSaturationEnabled() const;
        int saturationJitter() const;
        KisDynamicSensor* saturationSensor() const;
        bool isBrightnessEnabled() const;
        int brightnessJitter() const;
        KisDynamicSensor* brightnessSensor() const;
    public slots:
        void setMixerEnable(bool );
        void setMixerJitter(int );
        void setMixerSensor(KisDynamicSensor* );
        void setHueEnable(bool );
        void setHueJitter(int );
        void setHueSensor(KisDynamicSensor* );
        void setSaturationEnable(bool );
        void setSaturationJitter(int );
        void setSaturationSensor(KisDynamicSensor* );
        void setBrightnessEnable(bool );
        void setBrightnessJitter(int );
        void setBrightnessSensor(KisDynamicSensor* );
    private:
        bool m_mixerEnabled;
        int m_mixerJitter;
        KisDynamicSensor* m_mixerSensor;
        bool m_hueEnabled;
        int m_hueJitter;
        KisDynamicSensor* m_hueSensor;
        bool m_saturationEnabled;
        int m_saturationJitter;
        KisDynamicSensor* m_saturationSensor;
        bool m_brightnessEnabled;
        int m_brightnessJitter;
        KisDynamicSensor* m_brightnessSensor;
};

class KisBasicDynamicColoringProgramFactory : public KisDynamicColoringProgramFactory {
    public:
        KisBasicDynamicColoringProgramFactory();
        virtual KisDynamicColoringProgram* coloringProgram(QString name) const;
};

#endif
