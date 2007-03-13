/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_FILTERS_LIST_DYNAMIC_PROGRAM_H_
#define _KIS_FILTERS_LIST_DYNAMIC_PROGRAM_H_

#include "filterslistprogram_export.h"

#include "kis_dynamic_program.h"

#include <QList>

class KisDynamicTransformation;

class FILTERS_LIST_PROGRAM_EXPORT KisFiltersListDynamicProgram : public KisDynamicProgram {
    public:
        KisFiltersListDynamicProgram(const QString& name) : KisDynamicProgram(name)
        {
        }
        ~KisFiltersListDynamicProgram();
        virtual void apply(KisDynamicShape* shape, KisDynamicColoring* coloringsrc, const KisPaintInformation& adjustedInfo);
        inline QList<KisDynamicTransformation*>::iterator beginTransformation() { return m_transformations.begin(); }
        inline QList<KisDynamicTransformation*>::iterator endTransformation() { return m_transformations.end(); }
        inline KisDynamicTransformation* transfoAt(uint i) { return m_transformations[i]; }
        inline void removeTransformationAt(uint i) { m_transformations.removeAt(i); }
        inline uint countTransformations() const { return m_transformations.size(); }
        inline void appendTransformation(KisDynamicTransformation* transfo) {
            m_transformations.append(transfo);
        }
        virtual QWidget* createEditor(QWidget* parent);
    private:
        QList<KisDynamicTransformation*> m_transformations;
};

#endif
