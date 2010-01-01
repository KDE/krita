/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_brush_option.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_properties_configuration.h"

void KisBrushOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    if(!m_brush)
        return;
    
    QDomDocument d("BrushSetting");
    QDomElement e = d.createElement( "brush_definition" );
    m_brush->toXML( d, e );
    d.appendChild(e);
    setting->setProperty( "brush_definition", d.toString() );
}

void KisBrushOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    QString brushDefinition = setting->getString("brush_definition");
    QDomDocument d;
    d.setContent( brushDefinition, false );
    QDomElement e = d.elementsByTagName("brush_definition" ).at( 0 ).toElement();
    m_brush = KisBrush::fromXML( e );
}

KisBrushSP KisBrushOption::brush() const
{
    return m_brush;
}

void KisBrushOption::setBrush(KisBrushSP brush)
{
    m_brush = brush;
}
