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

#include "kis_basic_dynamic_program.h"

#include <QDomNode>
#include <QWidget>

#include <klocale.h>

#include "kis_properties_configuration.h"

// Dynamic Brush lib includes
#include "kis_dynamic_coloring.h"
#include "kis_dynamic_program_factory_registry.h"
#include "kis_dynamic_shape.h"
#include "kis_dynamic_transformation.h"
#include "kis_dynamic_transformations_factory.h"

// basic program includes
#include "kis_basic_dynamic_program_editor.h"


class Factory {
    public:
        Factory()
        {
            KisDynamicProgramFactoryRegistry::instance()->add( new KisBasicDynamicProgramFactory );
        }
};

static Factory factory;

KisBasicDynamicProgram::~KisBasicDynamicProgram()
{
}

void KisBasicDynamicProgram::apply(KisDynamicShape* shape, KisDynamicColoring* coloringsrc, const KisPaintInformation& adjustedInfo)
{

}

void KisBasicDynamicProgram::appendTransformation(KisDynamicTransformation* transfo) {
    kDebug(41006) << "Append transfo : " << transfo->name();
    m_transformations.append(transfo);
    emit(programChanged());
}

QWidget* KisBasicDynamicProgram::createEditor(QWidget* /*parent*/)
{
    return new KisBasicDynamicProgramEditor(this);
}

void KisBasicDynamicProgram::fromXML(const QDomElement& elt)
{
    QDomNode n = elt.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (not e.isNull()) {
            if( e.tagName() == "params")
            {
                KisPropertiesConfiguration kpc;
                kpc.fromXML(e);
                setEnableSize(kpc.getBool( "sizeEnabled", false) );
                setSizeMinimum(kpc.getDouble( "sizeMinimum", 0.0) );
                setSizeMaximum(kpc.getDouble( "sizeMaximum", 2.0) );
                setSizeJitter(kpc.getInt( "sizeJitter", 0) );
                setEnableAngle(kpc.getBool( "angleEnabled", false) );
                setAngleJitter(kpc.getInt( "angleJitter", 0) );
                setEnableScatter(kpc.getBool( "scatterEnabled", false) );
                setScatterJitter(kpc.getInt( "scatterJitter", 0) );
                setScatterAmount(kpc.getInt( "scatterAmount", 0) );
                setEnableCount(kpc.getInt( "countEnabled", 0) );
                setCountCount(kpc.getInt( "countCount", 0) );
                setCountJitter(kpc.getInt( "countJitter", 0) );
            }
        }
        n = n.nextSibling();
    }
    KisDynamicProgram::fromXML(elt);
}

void KisBasicDynamicProgram::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration kpc;
    kpc.setProperty("sizeEnabled", QVariant(isSizeEnabled()));
    kpc.setProperty("sizeMinimum", QVariant(sizeMinimum()));
    kpc.setProperty("sizeMaximum", QVariant(sizeMaximum()));
    kpc.setProperty("sizeJitter", QVariant(sizeJitter()));
    kpc.setProperty("angleEnabled", QVariant(isAngleEnabled()));
    kpc.setProperty("angleJitter", QVariant(angleJitter()));
    kpc.setProperty("scatterEnabled", QVariant(isScatterEnabled()));
    kpc.setProperty("scatterAmount", QVariant(scatterAmount()));
    kpc.setProperty("scatterJitter", QVariant(scatterJitter()));
    kpc.setProperty("isCountEnabled", QVariant(isCountEnabled()));
    kpc.setProperty("countCount", QVariant(countCount()));
    kpc.setProperty("countJitter", QVariant(countJitter()));
    QDomElement paramsElt = doc.createElement( "params" );
    rootElt.appendChild( paramsElt );
    kpc.toXML( doc, paramsElt);
    KisDynamicProgram::toXML(doc, rootElt);
}

bool KisBasicDynamicProgram::isSizeEnabled() const
{
    return m_sizeEnabled;
}

void KisBasicDynamicProgram::setEnableSize(bool v)
{
    m_sizeEnabled = v;
    emit(programChanged());
}

int KisBasicDynamicProgram::sizeMinimum() const
{
    return m_sizeMinimum;
}

void KisBasicDynamicProgram::setSizeMinimum(int min)
{
    m_sizeMinimum = min;
    emit(programChanged());
}

int KisBasicDynamicProgram::sizeMaximum() const
{
    return m_sizeMaximum;
}

void KisBasicDynamicProgram::setSizeMaximum(int max)
{
    m_sizeMaximum = max;
    emit(programChanged());
}

int KisBasicDynamicProgram::sizeJitter() const
{
    return m_sizeJitter;
}
void KisBasicDynamicProgram::setSizeJitter(int sj)
{
    m_sizeJitter = sj;
    emit(programChanged());
}

bool KisBasicDynamicProgram::isAngleEnabled() const
{
    return m_angleEnabled;
}

void KisBasicDynamicProgram::setEnableAngle(bool v)
{
    m_angleEnabled = v;
    emit(programChanged());
}

int KisBasicDynamicProgram::angleJitter() const
{
    return m_angleJitter;
}

void KisBasicDynamicProgram::setAngleJitter(int aj)
{
    m_angleJitter = aj;
    emit(programChanged());
}

bool KisBasicDynamicProgram::isScatterEnabled() const
{
    return m_scatterEnabled;
}

void KisBasicDynamicProgram::setEnableScatter(bool es)
{
    m_scatterEnabled = es;
    emit(programChanged());
}


int KisBasicDynamicProgram::scatterAmount() const
{
    return m_scatterAmount;
}

void KisBasicDynamicProgram::setScatterAmount(int sa)
{
    m_scatterAmount = sa;
    emit(programChanged());
}

int KisBasicDynamicProgram::scatterJitter() const
{
    return m_scatterJitter;
}

void KisBasicDynamicProgram::setScatterJitter(int sj)
{
    m_scatterJitter = sj;
    emit(programChanged());
}

bool KisBasicDynamicProgram::isCountEnabled() const
{
    return m_enableCout;
}

void KisBasicDynamicProgram::setEnableCount(bool ec)
{
    m_enableCout = ec;
    emit(programChanged());
}

int KisBasicDynamicProgram::countCount() const
{
    return m_countCount;
}

void KisBasicDynamicProgram::setCountCount(int cc)
{
    m_countCount = cc;
    emit(programChanged());
}

int KisBasicDynamicProgram::countJitter() const
{
    return m_countJitter;
}

void KisBasicDynamicProgram::setCountJitter(int cj)
{
    m_countJitter = cj;
    emit(programChanged());
}

//--- KisBasicDynamicProgramFactory ---//

KisBasicDynamicProgramFactory::KisBasicDynamicProgramFactory() :
    KisDynamicProgramFactory("basic", i18n("Basic"))
{
}

KisDynamicProgram* KisBasicDynamicProgramFactory::program(QString name)
{
    return new KisBasicDynamicProgram(name);
}

#include "kis_basic_dynamic_program.moc"
