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
};

class KisBasicDynamicColoringProgramFactory : public KisDynamicColoringProgramFactory {
    public:
        KisBasicDynamicColoringProgramFactory();
        virtual KisDynamicColoringProgram* coloringProgram(QString name) const;
};

#endif
