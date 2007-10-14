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

#include "kis_basic_dynamic_coloring_program.h"

#include <QDomNode>

#include "kis_properties_configuration.h"

// Dynamic Brush lib includes
#include "kis_dynamic_coloring.h"
#include "kis_dynamic_coloring_program_factory_registry.h"
#include "kis_dynamic_sensor.h"

// basic program includes
#include "kis_basic_dynamic_coloring_program_editor.h"

class Factory {
    public:
        Factory()
        {
            KisDynamicColoringProgramFactoryRegistry::instance()->add( new KisBasicDynamicColoringProgramFactory );
        }
};

static Factory factory;

KisBasicDynamicColoringProgram::KisBasicDynamicColoringProgram(const QString& name) :
        KisDynamicColoringProgram(name, "basiccoloring"),
        m_mixerEnabled(false),
        m_mixerJitter(0),
        m_mixerSensor(0),
        m_hueEnabled(false),
        m_hueJitter(0),
        m_hueSensor(0),
        m_saturationEnabled(false),
        m_saturationJitter(0),
        m_saturationSensor(0),
        m_brightnessEnabled(false),
        m_brightnessJitter(0),
        m_brightnessSensor(0)

{
}

KisBasicDynamicColoringProgram::~KisBasicDynamicColoringProgram()
{
}

void KisBasicDynamicColoringProgram::apply(KisDynamicColoring* coloring, const KisPaintInformation& adjustedInfo) const
{
}

QWidget* KisBasicDynamicColoringProgram::createEditor(QWidget* parent)
{
    return new KisBasicDynamicColoringProgramEditor(this);
}

void KisBasicDynamicColoringProgram::fromXML(const QDomElement& elt)
{
    QDomNode n = elt.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (not e.isNull()) {
            if( e.tagName() == "params")
            {
                KisPropertiesConfiguration kpc;
                kpc.fromXML(e);
                setMixerEnable(kpc.getBool( "mixerEnabled", false) );
                setMixerJitter(kpc.getInt( "mixerJitter", 0) );
                setHueEnable(kpc.getBool( "hueEnabled", false) );
                setHueJitter(kpc.getInt( "hueJitter", 0) );
                setSaturationEnable(kpc.getBool( "saturationEnabled", false) );
                setSaturationJitter(kpc.getInt( "saturationJitter", 0) );
                setBrightnessEnable(kpc.getBool( "brightnessEnabled", false) );
                setBrightnessJitter(kpc.getInt( "brightnessJitter", 0) );
            } else if (e.tagName() == "mixerSensor") {
                m_mixerSensor = KisDynamicSensor::createFromXML(e);
            } else if (e.tagName() == "hueSensor") {
                m_hueSensor = KisDynamicSensor::createFromXML(e);
            } else if (e.tagName() == "saturationSensor") {
                m_saturationSensor = KisDynamicSensor::createFromXML(e);
            } else if (e.tagName() == "brightnessSensor") {
                m_brightnessSensor = KisDynamicSensor::createFromXML(e);
            }
        }
        n = n.nextSibling();
    }
    KisDynamicColoringProgram::fromXML(elt);
}

void KisBasicDynamicColoringProgram::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration kpc;
    kpc.setProperty("mixerEnabled", QVariant(isMixerEnabled()));
    kpc.setProperty("mixerJitter", QVariant(mixerJitter()));
    kpc.setProperty("hueEnabled", QVariant(isHueEnabled()));
    kpc.setProperty("hueJitter", QVariant(hueJitter()));
    kpc.setProperty("saturationEnabled", QVariant(isSaturationEnabled()));
    kpc.setProperty("saturationJitter", QVariant(saturationJitter()));
    kpc.setProperty("brightnessEnabled", QVariant(isBrightnessEnabled()));
    kpc.setProperty("brightnessJitter", QVariant(brightnessJitter()));
    QDomElement paramsElt = doc.createElement( "params" );
    rootElt.appendChild( paramsElt );
    kpc.toXML( doc, paramsElt);
    if(m_mixerSensor)
    {
        QDomElement eSensor = doc.createElement( "mixerSensor" );
        m_mixerSensor->toXML( doc, eSensor);
        rootElt.appendChild( eSensor );
    }
    if(m_hueSensor)
    {
        QDomElement eSensor = doc.createElement( "hueSensor" );
        m_hueSensor->toXML( doc, eSensor);
        rootElt.appendChild( eSensor );
    }
    if(m_saturationSensor)
    {
        QDomElement eSensor = doc.createElement( "saturationSensor" );
        m_saturationSensor->toXML( doc, eSensor);
        rootElt.appendChild( eSensor );
    }
    if(m_brightnessSensor)
    {
        QDomElement eSensor = doc.createElement( "brightnessSensor" );
        m_brightnessSensor->toXML( doc, eSensor);
        rootElt.appendChild( eSensor );
    }

    KisDynamicColoringProgram::toXML(doc, rootElt);
}

// Mixer

bool KisBasicDynamicColoringProgram::isMixerEnabled() const
{
    return m_mixerEnabled;
}

void KisBasicDynamicColoringProgram::setMixerEnable(bool v)
{
    m_mixerEnabled = v;
    emit(programChanged());
}

int KisBasicDynamicColoringProgram::mixerJitter() const
{
    return m_mixerJitter;
}
void KisBasicDynamicColoringProgram::setMixerJitter(int sj)
{
    m_mixerJitter = sj;
    emit(programChanged());
}

KisDynamicSensor* KisBasicDynamicColoringProgram::mixerSensor() const
{
    return m_mixerSensor;
}
void KisBasicDynamicColoringProgram::setMixerSensor(KisDynamicSensor* s)
{
    m_mixerSensor = s;
    emit(programChanged());
}

// Hue

bool KisBasicDynamicColoringProgram::isHueEnabled() const
{
    return m_hueEnabled;
}

void KisBasicDynamicColoringProgram::setHueEnable(bool v)
{
    m_hueEnabled = v;
    emit(programChanged());
}

int KisBasicDynamicColoringProgram::hueJitter() const
{
    return m_hueJitter;
}
void KisBasicDynamicColoringProgram::setHueJitter(int sj)
{
    m_hueJitter = sj;
    emit(programChanged());
}

KisDynamicSensor* KisBasicDynamicColoringProgram::hueSensor() const
{
    return m_hueSensor;
}
void KisBasicDynamicColoringProgram::setHueSensor(KisDynamicSensor* s)
{
    m_hueSensor = s;
    emit(programChanged());
}

// Saturation

bool KisBasicDynamicColoringProgram::isSaturationEnabled() const
{
    return m_saturationEnabled;
}

void KisBasicDynamicColoringProgram::setSaturationEnable(bool v)
{
    m_saturationEnabled = v;
    emit(programChanged());
}

int KisBasicDynamicColoringProgram::saturationJitter() const
{
    return m_saturationJitter;
}
void KisBasicDynamicColoringProgram::setSaturationJitter(int sj)
{
    m_saturationJitter = sj;
    emit(programChanged());
}

KisDynamicSensor* KisBasicDynamicColoringProgram::saturationSensor() const
{
    return m_saturationSensor;
}
void KisBasicDynamicColoringProgram::setSaturationSensor(KisDynamicSensor* s)
{
    m_saturationSensor = s;
    emit(programChanged());
}

// Brightness

bool KisBasicDynamicColoringProgram::isBrightnessEnabled() const
{
    return m_brightnessEnabled;
}

void KisBasicDynamicColoringProgram::setBrightnessEnable(bool v)
{
    m_brightnessEnabled = v;
    emit(programChanged());
}

int KisBasicDynamicColoringProgram::brightnessJitter() const
{
    return m_brightnessJitter;
}
void KisBasicDynamicColoringProgram::setBrightnessJitter(int sj)
{
    m_brightnessJitter = sj;
    emit(programChanged());
}

KisDynamicSensor* KisBasicDynamicColoringProgram::brightnessSensor() const
{
    return m_brightnessSensor;
}
void KisBasicDynamicColoringProgram::setBrightnessSensor(KisDynamicSensor* s)
{
    m_brightnessSensor = s;
    emit(programChanged());
}

//--- KisBasicDynamicColoringProgramFactory ---//

KisBasicDynamicColoringProgramFactory::KisBasicDynamicColoringProgramFactory() :
    KisDynamicColoringProgramFactory("basiccoloring", i18n("Basic"))
{
}

KisDynamicColoringProgram* KisBasicDynamicColoringProgramFactory::coloringProgram(QString name) const
{
    return new KisBasicDynamicColoringProgram(name);
}

#include "kis_basic_dynamic_coloring_program.moc"
