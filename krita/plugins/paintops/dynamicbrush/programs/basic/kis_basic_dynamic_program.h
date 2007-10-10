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

class KisDynamicTransformation;

class KisBasicDynamicProgram : public KisDynamicProgram {
    Q_OBJECT
    public:
        KisBasicDynamicProgram(const QString& name);
        ~KisBasicDynamicProgram();
        virtual void apply(KisDynamicShape* shape, KisDynamicColoring* coloringsrc, const KisPaintInformation& adjustedInfo);
        inline QList<KisDynamicTransformation*>::iterator beginTransformation() { return m_transformations.begin(); }
        inline QList<KisDynamicTransformation*>::iterator endTransformation() { return m_transformations.end(); }
        inline KisDynamicTransformation* transfoAt(uint i) { return m_transformations[i]; }
        inline void removeTransformationAt(uint i) { m_transformations.removeAt(i); }
        inline uint countTransformations() const { return m_transformations.size(); }
        void appendTransformation(KisDynamicTransformation* transfo);
        virtual QWidget* createEditor(QWidget* parent);
        virtual void fromXML(const QDomElement&);
        virtual void toXML(QDomDocument&, QDomElement&) const;
    public:
        bool isSizeEnabled() const;
        int sizeMinimum() const;
        int sizeMaximum() const;
        int sizeJitter() const;
        bool isAngleEnabled() const;
        int angleJitter() const;
        bool isScatterEnabled() const;
        int scatterAmount() const;
        int scatterJitter() const;
        bool isCountEnabled() const;
        int countCount() const;
        int countJitter() const;
    public slots:
        void setEnableSize(bool );
        void setSizeMinimum(int );
        void setSizeMaximum(int );
        void setSizeJitter(int );
        void setEnableAngle(bool );
        void setAngleJitter(int );
        void setEnableScatter(bool );
        void setScatterAmount(int );
        void setScatterJitter(int );
        void setEnableCount(bool );
        void setCountCount(int );
        void setCountJitter(int );
    private:
        QList<KisDynamicTransformation*> m_transformations;
        bool m_sizeEnabled;
        int m_sizeMinimum;
        int m_sizeMaximum;
        int m_sizeJitter;
        bool m_angleEnabled;
        int m_angleJitter;
        int m_scatterEnabled;
        int m_scatterAmount;
        int m_scatterJitter;
        bool m_enableCout;
        int m_countCount;
        int m_countJitter;
};

class KisBasicDynamicProgramFactory : public KisDynamicProgramFactory {
    public:
        KisBasicDynamicProgramFactory();
        virtual KisDynamicProgram* program(QString name);
};

#endif
