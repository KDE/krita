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

#ifndef _KIS_BASIC_DYNAMIC_PROGRAM_H_
#define _KIS_BASIC_DYNAMIC_PROGRAM_H_

#include "kis_dynamic_program.h"

#include <QList>

class KisDynamicSensor;

class KisBasicDynamicProgram : public KisDynamicProgram {
    Q_OBJECT
    public:
        KisBasicDynamicProgram(const QString& name);
        ~KisBasicDynamicProgram();
        virtual void apply(KisDynamicShape* shape, KisDynamicColoring* coloringsrc, const KisPaintInformation& adjustedInfo);
        virtual QWidget* createEditor(QWidget* parent);
        virtual void fromXML(const QDomElement&);
        virtual void toXML(QDomDocument&, QDomElement&) const;
    public:
        bool isSizeEnabled() const;
        int sizeMinimum() const;
        int sizeMaximum() const;
        int sizeJitter() const;
        KisDynamicSensor* sizeSensor() const;
        bool isAngleEnabled() const;
        int angleJitter() const;
        KisDynamicSensor* angleSensor() const;
        bool isScatterEnabled() const;
        int scatterAmount() const;
        int scatterJitter() const;
        KisDynamicSensor* scatterSensor() const;
        bool isCountEnabled() const;
        int countCount() const;
        int countJitter() const;
        KisDynamicSensor* countSensor() const;
    public slots:
        void setEnableSize(bool );
        void setSizeMinimum(int );
        void setSizeMaximum(int );
        void setSizeJitter(int );
        void setSizeSensor(KisDynamicSensor* );
        void setEnableAngle(bool );
        void setAngleJitter(int );
        void setAngleSensor(KisDynamicSensor* );
        void setEnableScatter(bool );
        void setScatterAmount(int );
        void setScatterJitter(int );
        void setScatterSensor(KisDynamicSensor* );
        void setEnableCount(bool );
        void setCountCount(int );
        void setCountJitter(int );
        void setCountSensor(KisDynamicSensor* );
    private:
        bool m_sizeEnabled;
        int m_sizeMinimum;
        int m_sizeMaximum;
        int m_sizeJitter;
        KisDynamicSensor* m_sizeSensor;
        bool m_angleEnabled;
        int m_angleJitter;
        KisDynamicSensor* m_angleSensor;
        bool m_scatterEnabled;
        int m_scatterAmount;
        int m_scatterJitter;
        KisDynamicSensor* m_scatterSensor;
        bool m_enableCout;
        int m_countCount;
        int m_countJitter;
        KisDynamicSensor* m_countSensor;
};

class KisBasicDynamicProgramFactory : public KisDynamicProgramFactory {
    public:
        KisBasicDynamicProgramFactory();
        virtual KisDynamicProgram* program(QString name);
};

#endif
