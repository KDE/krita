/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_abr_translator.h"
#include <kis_debug.h>
#include <QStringList>
#include <kis_dom_utils.h>

KisAbrTranslator::KisAbrTranslator()
{
    init();
}

KisAbrTranslator::~KisAbrTranslator()
{

}

void KisAbrTranslator::init()
{
    m_root = m_doc.createElement("Preset");
    m_root.setAttribute("paintopid", "paintbrush");
}

void KisAbrTranslator::addEntry(const QString& attributeName, const QString& type, const QString& value)
{
    // setup object type
    // shape dynamics is not separated in the Objc so workaround by attribute name
    if (type == ABR_OBJECT ||
            attributeName == ABR_USE_TIP_DYNAMICS ||
            attributeName == ABR_USE_SCATTER) {

        if (m_currentObjectName == ABR_DUAL_BRUSH && attributeName == OBJECT_NAME_BRUSH) {
            m_currentObjectName = ABR_DUAL_BRUSH + '_' + attributeName;
        }
        else {
            m_currentObjectName = attributeName;
        }

        dbgKrita << "---- Object type changed to " << m_currentObjectName;
    }

    // behaviour according object's attribute name
    if (m_currentObjectName == ABR_PRESET_START) {
        if (attributeName == ABR_PRESET_START) {
            clean();
        }
        else if (attributeName == ABR_PRESET_NAME) {
            m_root.setAttribute("name", value);
        }
        else {
            //dbgKrita << "--Unknown attribute: " << attributeName;
        }
    }
    else if (m_currentObjectName == OBJECT_NAME_BRUSH) {
        m_abrBrushProperties.setupProperty(attributeName, type, value);
        // this is not object name but shape dynamics does not start with objc :/
        // those objects has been merged with shape attributes due to serialization
        // e.g. minumumDiameter belongs to ABR_SZVR and is ABR_USE_TIP_DYNAMICS object
    }
    else if (m_currentObjectName == ABR_USE_TIP_DYNAMICS ||
               m_currentObjectName == ABR_SZVR ||
               m_currentObjectName == ABR_ANGLE_DYNAMICS ||
               m_currentObjectName == ABR_ROUNDNESS_DYNAMICS) {
        m_abrTipDynamics.setupProperty(attributeName, type, value);
    }
    else if (m_currentObjectName == ABR_USE_SCATTER) {
        // TODO
    }
    else {
        dbgKrita << "Unknown attribute of " << m_currentObjectName << "| " << attributeName << type << value << " |";
    }



}

void KisAbrTranslator::finishPreset()
{
    m_abrBrushProperties.toXML(m_doc, m_root);
    m_abrTipDynamics.toXML(m_doc, m_root);
    m_doc.appendChild(m_root);

    m_abrTipDynamics.reset();
    m_currentObjectName.clear();
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
            attributeName == OBJECT_NAME_BRUSH) {
        list = value.split(' ');
        //e.g. "#Pxl 10" -> ['#Pxl','10']
        Q_ASSERT(list.count() == 2);
        valueDbl = KisDomUtils::toDouble(list.at(1));
    }

    if (attributeName == OBJECT_NAME_BRUSH) {
        m_brushType = list.at(0);
    }
    else if (attributeName == ABR_BRUSH_DIAMETER) {
        m_diameter = valueDbl;
    }
    else if (attributeName == ABR_BRUSH_HARDNESS) {
        m_hardness = valueDbl;
    }
    else if (attributeName == ABR_BRUSH_ANGLE) {
        m_angle = valueDbl;
    }
    else if (attributeName == ABR_BRUSH_ROUNDNESS) {
        m_roundness = valueDbl;
    }
    else if (attributeName == ABR_BRUSH_SPACING) {
        m_spacing = valueDbl;
    }
    else if (attributeName == ABR_BRUSH_INTR) {
        m_intr = value.toInt();
    }
    else if (attributeName == ABR_FLIP_X) {
        m_flipX = value.toInt();
    }
    else if (attributeName == ABR_FLIP_Y) {
        m_flipY = value.toInt();
    }
    else {
        dbgKrita << "Unknown attribute " << attributeName;
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
    if (m_brushType != BRUSH_TYPE_COMPUTED) {
        dbgKrita << m_brushType << "saved as computed brush...";
    }

    QDomDocument d;
    QDomElement e = d.createElement("Brush");

    QDomElement shapeElement = d.createElement("MaskGenerator");
    shapeElement.setAttribute("radius", KisDomUtils::toString(m_diameter * 0.5)); // radius == diameter / 2
    shapeElement.setAttribute("ratio", KisDomUtils::toString(m_roundness / 100.0)); // roundness in (0..100) to ratio in (0.0..1.0)
    shapeElement.setAttribute("hfade", KisDomUtils::toString(m_hardness / 100.0)); // same here
    shapeElement.setAttribute("vfade", KisDomUtils::toString(m_hardness / 100.0)); // and here too
    shapeElement.setAttribute("spikes", KisDomUtils::toString(2)); // just circle so far
    shapeElement.setAttribute("type", "circle");
    e.appendChild(shapeElement);

    e.setAttribute("type", "auto_brush");
    e.setAttribute("spacing", KisDomUtils::toString(m_spacing / 100.0)); // spacing from 0..1000 to
    e.setAttribute("angle", KisDomUtils::toString(m_angle < 0 ? m_angle + 360.0 : m_angle)); // angle from -180..180 to 0..360
    e.setAttribute("randomness", "0");  // default here
    d.appendChild(e);

    QDomElement elementParam = doc.createElement("param");
    elementParam.setAttribute("name", "brush_definition");
    QDomText text = doc.createCDATASection(d.toString());
    elementParam.appendChild(text);
    root.appendChild(elementParam);

}

AbrTipDynamicsProperties::AbrTipDynamicsProperties()
{
    m_groups[ ABR_SZVR ] = &m_sizeVarianceProperties;
    m_groups[ ABR_ANGLE_DYNAMICS ] = &m_angleProperties;
    m_groups[ ABR_ROUNDNESS_DYNAMICS ] = &m_RoundnessProperties;
    Q_ASSERT(m_groupType.isNull());
}


void AbrTipDynamicsProperties::setupProperty(const QString& attributeName, const QString& type, const QString& value)
{
    if (type == ABR_OBJECT) {
        if (!m_groups.contains(attributeName)) {
            dbgKrita << "Unknown " << type << " in Tip dynamics called " << attributeName << " : " << value;
        }
        else {
            m_groupType = attributeName;
        }
        return;
    }

    Q_UNUSED(type);
    double valueDbl = 0.0;
    QStringList list;
    if (attributeName == ABR_TIP_DYNAMICS_MINUMUM_DIAMETER ||
            attributeName == ABR_TIP_DYNAMICS_MINUMUM_ROUNDNESS ||
            attributeName == ABR_TIP_DYNAMICS_TILT_SCALE
       ) {
        list = value.split(' ');
        //e.g. "#Pxl 10" -> ['#Pxl','10']
        Q_ASSERT(list.count() == 2);
        valueDbl = KisDomUtils::toDouble(list.at(1));
    }

    if (m_groupType.isNull()) {

        if (attributeName == ABR_USE_TIP_DYNAMICS) {
            m_useTipDynamics = value.toInt();
        }
        else if (attributeName == ABR_FLIP_X) {
            m_flipX = value.toInt();
        }
        else if (attributeName == ABR_FLIP_Y) {
            m_flipY = value.toInt();
        }
        else if (attributeName == ABR_TIP_DYNAMICS_MINUMUM_DIAMETER) {
            m_minumumDiameter = valueDbl;
        }
        else if (attributeName == ABR_TIP_DYNAMICS_MINUMUM_ROUNDNESS) {
            m_minumumRoundness = valueDbl;
        }
        else if (attributeName == ABR_TIP_DYNAMICS_TILT_SCALE) {
            m_tiltScale = valueDbl;
        }
        else {
            dbgKrita << "Unknown attribute for tip dynamics" << attributeName;
        }

    } else {
        m_groups[ m_groupType ]->setupProperty(attributeName, type, value);
    }
}

void AbrTipDynamicsProperties::toXML(QDomDocument& doc, QDomElement& root) const
{
    QDomElement el = doc.createElement("shape_dynamics");
    el.setAttribute("useTipDynamics", m_useTipDynamics);
    el.setAttribute("flipX", m_flipX);
    el.setAttribute("flipY", m_flipY);

    root.appendChild(el);

    el = doc.createElement("angleDynamics");

    el.setAttribute("angleJitter", KisDomUtils::toString(m_angleProperties.m_sizeJitter));
    el.setAttribute("angleController", KisDomUtils::toString(m_angleProperties.m_bVTy));
    el.setAttribute("angleFadeStep", KisDomUtils::toString(m_angleProperties.m_fadeStep));
    root.appendChild(el);

    el = doc.createElement("roundnessDynamics");
    el.setAttribute("minumumRoundness", KisDomUtils::toString(m_minumumRoundness));
    el.setAttribute("roundnessJitter", KisDomUtils::toString(m_RoundnessProperties.m_sizeJitter));
    el.setAttribute("roundnessController", KisDomUtils::toString(m_RoundnessProperties.m_bVTy));
    el.setAttribute("roundnessFadeStep", KisDomUtils::toString(m_RoundnessProperties.m_fadeStep));
    root.appendChild(el);

    el = doc.createElement("sizeDynamics");
    el.setAttribute("tiltScale", KisDomUtils::toString(m_tiltScale));
    el.setAttribute("minumumDiameter", KisDomUtils::toString(m_minumumDiameter));
    el.setAttribute("roundnessJitter", KisDomUtils::toString(m_RoundnessProperties.m_sizeJitter));
    el.setAttribute("roundnessController", KisDomUtils::toString(m_RoundnessProperties.m_bVTy));
    el.setAttribute("roundnessFadeStep", KisDomUtils::toString(m_RoundnessProperties.m_fadeStep));
    root.appendChild(el);

}

// <param name="CurveSize"><![CDATA[0,0.257028;1,0.493976;]]></param>
//  0,m_minimumDiameter      1,m_sizeJitter
// <param name="PressureSize">true</param>
// <param name="SizeSensor"><![CDATA[<!DOCTYPE params>
// <params id="fuzzy"/> ]]></param> // controller
void AbrGroupProperties::setupProperty(const QString& attributeName, const QString& type, const QString& value)
{
    Q_UNUSED(type);
    double valueDbl = 0.0;
    QStringList list;
    if (attributeName == ABR_DYNAMICS_JITTER) {
        list = value.split(' ');
        //e.g. "#Pxl 10" -> ['#Pxl','10']
        Q_ASSERT(list.count() == 2);
        valueDbl = KisDomUtils::toDouble(list.at(1));
    }

    if (attributeName == ABR_DYNAMICS_FADE_STEP) {
        m_fadeStep = value.toInt();
    }
    else if (attributeName == ABR_DYNAMICS_JITTER) {
        m_sizeJitter = valueDbl;
    }
    else if (attributeName == ABR_CONTROL) {
        m_bVTy = (enumAbrControllers)value.toInt();
    }
    else {
        dbgKrita << "Unknown attribute for Group!" << attributeName;
    }

}



