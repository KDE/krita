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

#ifndef _KIS_DYNAMIC_COLORING_PROGRAM_H_
#define _KIS_DYNAMIC_COLORING_PROGRAM_H_

#include "dynamicbrush_export.h"

#include "kis_dynamic_program.h"

#include <QString>
#include <QObject>

class KisDynamicColoring;
class KisPaintInformation;
class QWidget;

/**
 * This is the base class of a dynamic program.
 */
class DYNAMIC_BRUSH_EXPORT KisDynamicColoringProgram : public KisDynamicProgram {
    Q_OBJECT
    protected:
        /**
         * @param name the name of this program, will be displayed in the list of programs
         */
        KisDynamicColoringProgram(const QString& name, const QString& type);
    public:
        virtual ~KisDynamicColoringProgram();
        /**
         * Apply the program on the shape and the coloring
         */
        virtual void apply(KisDynamicColoring* coloringsrc, const KisPaintInformation& ) const = 0;
    private:
        struct Private;
        Private* const d;
};

class DYNAMIC_BRUSH_EXPORT KisDynamicColoringProgramFactory : public KisDynamicProgramFactory {
    public:
        KisDynamicColoringProgramFactory(QString id, QString name);
        virtual ~KisDynamicColoringProgramFactory();
        virtual KisDynamicProgram* program(QString name) const;
        virtual KisDynamicColoringProgram* coloringProgram(QString name) const = 0;
    private:
        struct Private;
        Private* const d;
};

class DYNAMIC_BRUSH_EXPORT KisDynamicColoringProgramsFactory : public KisSerializableConfigurationFactory {
    public:
        virtual ~KisDynamicColoringProgramsFactory();
        virtual KisSerializableConfiguration* createDefault();
        virtual KisSerializableConfiguration* create(const QDomElement&);
};

#endif
