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

#ifndef _KIS_DYNAMIC_PROGRAM_H_
#define _KIS_DYNAMIC_PROGRAM_H_

#include <QString>

class KisDynamicShape;
class KisDynamicColoring;
class KisPaintInformation;

/**
 * This is the base class of a dynamic program.
 */
class KisDynamicProgram {
    protected:
        KisDynamicProgram(const QString& name) : m_name(name) { }
        virtual ~KisDynamicProgram() {}
    public:
        virtual ~KisDynamicProgram() {}
        /// @return the name of this program
        const QString& name() { return m_name; }
        /**
         * Apply the program on the shape and the coloring
         */
        virtual void apply(KisDynamicShape* shape, KisDynamicColoring* coloringsrc, const KisPaintInformation& adjustedInfo) = 0;
    private:
        QString m_name;
};


#endif
