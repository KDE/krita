/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
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

#ifndef _KIS_DYNAMIC_BRUSH_HPP_
#define _KIS_DYNAMIC_BRUSH_HPP_

#include <QString>
#include <KoID.h>

class KisDynamicColoring;
class KisDynamicShape;
class KisDynamicTransformation;

class KisDynamicBrush {
    public:
        KisDynamicBrush(const QString& name);
        ~KisDynamicBrush();
        inline QString name() const { return m_name; }
        inline KisDynamicTransformation* rootTransfo() const { return m_rootTransfo; }
        inline KoID id() const { return KoID(name(), name()); }
        inline /*const */KisDynamicShape* shape() const { return m_shape; }
        inline /*const */KisDynamicColoring* coloring() const { return m_coloring; }
    private:
        QString m_name;
        KisDynamicTransformation* m_rootTransfo;
        KisDynamicShape* m_shape;
        KisDynamicColoring* m_coloring;
};

#endif
