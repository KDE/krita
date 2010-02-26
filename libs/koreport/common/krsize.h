/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KRSIZE_H
#define KRSIZE_H
#include <KoUnit.h>
#include <QSizeF>
#include <koproperty/Property.h>

/**
 @author
*/
class KRSize
{
public:
    KRSize(const KoUnit& unit = KoUnit(KoUnit::Centimeter));

    ~KRSize();
    QSizeF toUnit();
    QSizeF toPoint();
    QSizeF toScene();
    void setSceneSize(const QSizeF&, bool update= true);
    void setUnitSize(const QSizeF&, bool update = true);
    void setPointSize(const QSizeF&, bool update = true);
    void setUnit(KoUnit);

    KoProperty::Property* property() {
        return m_property;
    }
private:
    QSizeF m_pointSize;
    KoUnit m_unit;
    KoProperty::Property* m_property;
};

#endif
