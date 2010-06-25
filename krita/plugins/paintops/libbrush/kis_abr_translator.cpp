/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_abr_translator.h"
#include <QDebug>
#include <QStringList>

KisAbrTranslator::KisAbrTranslator()
{
    init();
}

KisAbrTranslator::~KisAbrTranslator()
{

}

void KisAbrTranslator::init()
{
    m_currentObjectName = "";
    m_root = m_doc.createElement("Preset");
    m_root.setAttribute("paintopid","spraybrush");
}

void KisAbrTranslator::addEntry(const QString& attributeName, const QString& type, const QString& value)
{
    // setup object type
    // shape dynamics is not separated in the Objc so workaround by attribute name
    if (type == ABR_OBJECT || attributeName == ABR_USE_TIP_DYNAMICS){ 
        m_currentObjectName = attributeName; 
        qDebug() << "Object type changed to " << m_currentObjectName;
    }
    
    // behaviour according object type    
    if (m_currentObjectName == ABR_PRESET_START){
        
        if (attributeName == ABR_PRESET_START){ 
            clean(); 
        }else 
        if (attributeName == ABR_PRESET_NAME)
        {
            m_root.setAttribute("name",value);
        }else{
            qDebug() << "--Unknown attribute: " << attributeName;
        }
    }else
    if (m_currentObjectName == OBJECT_NAME_BRUSH){
        m_abrBrushProperties.setupProperty(attributeName,type,value);
    }else{
            qDebug() << "+Unsupported object type " << m_currentObjectName;
    }
}

void KisAbrTranslator::finishPreset()
{
    m_abrBrushProperties.toXML(m_doc, m_root);
    m_doc.appendChild(m_root);
}


QString KisAbrTranslator::toString()
{
    return m_doc.toString();
}

void KisAbrTranslator::clean()
{
    m_doc.clear();
    m_root.clear();
    init();
}


void AbrBrushProperties::setupProperty(const QString& attributeName, const QString& type, const QString& value)
{
    Q_UNUSED(type);
    double valueDbl = 0.0;
    QStringList list;
    if (attributeName == ABR_BRUSH_DIAMETER ||
        attributeName == ABR_BRUSH_HARDNESS ||
        attributeName == ABR_BRUSH_ANGLE ||
        attributeName == ABR_BRUSH_ROUNDNESS ||
        attributeName == ABR_BRUSH_SPACING ||
        attributeName == OBJECT_NAME_BRUSH)
    {
        list = value.split(" ");
        //e.g. "#Pxl 10" -> ['#Pxl','10'] 
        Q_ASSERT(list.count() == 2);
        bool ok;
        valueDbl = list.at(1).toDouble(&ok);
        Q_ASSERT(ok);
    }

    if (attributeName == OBJECT_NAME_BRUSH){ m_brushType = list.at(0); }
    else if (attributeName == ABR_BRUSH_DIAMETER){ m_diameter = valueDbl; }
    else if (attributeName == ABR_BRUSH_HARDNESS){ m_hardness = valueDbl; }
    else if (attributeName == ABR_BRUSH_ANGLE){ m_angle = valueDbl; }
    else if (attributeName == ABR_BRUSH_ROUNDNESS){ m_roundness = valueDbl; }
    else if (attributeName == ABR_BRUSH_SPACING){ m_spacing = valueDbl; }
    else if (attributeName == ABR_BRUSH_INTR){ m_intr = value.toInt(); }
    else if (attributeName == ABR_BRUSH_FLIP_X) { m_flipX = value.toInt(); }
    else if (attributeName == ABR_BRUSH_FLIP_Y) { m_flipY = value.toInt(); }
    else {
        qDebug() << "Unknown attribute " << attributeName;
    }
}

// <param name="brush_definition">
//     <![CDATA[
//     <Brush type="auto_brush" spacing="0.1" angle="0"> 
//         <MaskGenerator radius="5" ratio="1" type="circle" vfade="0.5" spikes="2" hfade="0.5"/> 
//     </Brush> ]]>
// </param>
void AbrBrushProperties::toXML(QDomDocument& doc, QDomElement& root) const
{
    if (m_brushType != BRUSH_TYPE_COMPUTED){    
        qDebug() << m_brushType << "saved as computed brush...";
    }
    
    QDomDocument d;
    QDomElement e = d.createElement( "Brush" );

    QDomElement shapeElement = d.createElement("MaskGenerator");
    shapeElement.setAttribute("radius", (m_diameter * 0.5)); // radius == diameter / 2
    shapeElement.setAttribute("ratio", m_roundness / 100.0); // roundness in (0..100) to ratio in (0.0..1.0)
    shapeElement.setAttribute("hfade", m_hardness / 100.0); // same here 
    shapeElement.setAttribute("vfade", m_hardness / 100.0); // and here too
    shapeElement.setAttribute("spikes", 2); // just circle so far
    shapeElement.setAttribute("type", "circle"); 
    e.appendChild(shapeElement);

    e.setAttribute("type", "auto_brush");
    e.setAttribute("spacing", m_spacing / 100.0); // spacing from 0..1000 to 
    e.setAttribute("angle", m_angle < 0 ? m_angle + 360.0 : m_angle); // angle from -180..180 to 0..360
    e.setAttribute("randomness", 0);  // default here    
    d.appendChild(e);

    QDomElement elementParam = doc.createElement("param");
    elementParam.setAttribute("name","brush_definition");
    QDomText text = doc.createCDATASection(d.toString());
    elementParam.appendChild(text);
    root.appendChild(elementParam);

}
