/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#include "KoCtlColorProfile.h"

#include <kdebug.h>

#include <QDomDocument>
#include <QFile>

#include <KoID.h>

#include <GTLCore/PixelDescription.h>
#include <GTLCore/Type.h>
#include <OpenCTL/Program.h>
#include <OpenCTL/Module.h>

struct ConversionInfo {
    ConversionInfo( QString _function, KoID _sourceColorModelID, KoID _sourceColorDepthID,
                    KoID _destinationColorModelID, KoID _destinationColorDepthID) :
                    sourceColorModelID( _sourceColorModelID), sourceColorDepthID( _sourceColorDepthID),
                    destinationColorModelID( _destinationColorModelID), destinationColorDepthID( _destinationColorDepthID), function(_function)
    {
    }
    KoID sourceColorModelID;
    KoID sourceColorDepthID;
    KoID destinationColorModelID;
    KoID destinationColorDepthID;
    QString function;
};

struct KoCtlColorProfile::Private {
    OpenCTL::Module* module;
    QList<ConversionInfo> conversionInfos;
    KoID colorModelID;
    KoID colorDepthID;
};

KoCtlColorProfile::KoCtlColorProfile(QString filename) : KoColorProfile(filename), d(new Private)
{
    d->module = 0;
}

KoCtlColorProfile::KoCtlColorProfile(const KoCtlColorProfile& rhs) : KoColorProfile(rhs), d(new Private(*rhs.d))
{
    
}

KoCtlColorProfile::~KoCtlColorProfile()
{
    delete d;
}

KoColorProfile* KoCtlColorProfile::clone() const
{
    return new KoCtlColorProfile(*this);
}

bool KoCtlColorProfile::valid() const
{
    kDebug(/*DBG_PIGMENT*/) << d->colorModelID.id().isNull() << " " << d->colorDepthID.id().isNull();
    return ( d->module and d->module->isCompiled()
            and not d->colorModelID.id().isNull() and not d->colorDepthID.id().isNull());
}

bool KoCtlColorProfile::isSuitableForOutput() const
{
    return true;
}

bool KoCtlColorProfile::isSuitableForPrinting() const
{
    return true;
}

bool KoCtlColorProfile::isSuitableForDisplay() const
{
    return true;
}

OpenCTL::Program* KoCtlColorProfile::createColorConversionProgram(KoID _srcModelId, KoID _srcDepthId, KoID _dstModelId, KoID _dstDepthId) const
{
    foreach(ConversionInfo info, d->conversionInfos)
    {
        if(info.sourceColorModelID == _srcModelId and info.sourceColorDepthID == _srcDepthId and info.destinationColorModelID == _dstModelId and info.destinationColorDepthID == _dstDepthId)
        {
            return new OpenCTL::Program(info.function.toAscii().data(), d->module, GTLCore::PixelDescription(GTLCore::Type::Float, 4) );
        }
    }
    return 0;
}

bool KoCtlColorProfile::operator==(const KoColorProfile& p) const
{
    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(&p);
    if(ctlp)
    {
        return ctlp->name() == name() and ctlp->colorModel() == colorModel() and ctlp->colorDepth() == colorDepth();
    } else {
        return false;
    }
}

KoID KoCtlColorProfile::colorModel() const
{
    return d->colorModelID;
}

KoID KoCtlColorProfile::colorDepth() const
{
    return d->colorDepthID;
}

void KoCtlColorProfile::decodeTransformations(QDomElement& elt)
{
    for(QDomNode nt = elt.firstChild(); not nt.isNull(); nt = nt.nextSibling())
    {
        QDomElement et = nt.toElement();
        if(not et.isNull())
        {
            kDebug(/*DBG_PIGMENT*/) << et.tagName();
            if(et.tagName() == "conversions")
            {
                decodeConversions(et);
            }
        }
    }
}

KoID generateDepthID(QString depth, QString type)
{
    QString prefix;
    if(type == "integer") prefix = "U";
    else if(type == "float") prefix = "F";
    else kDebug(/*DBG_PIGMENT*/) << "Invalid type";
    return KoID(prefix + depth,"");
}

void KoCtlColorProfile::decodeConversions(QDomElement& elt)
{
    for(QDomNode n = elt.firstChild(); not n.isNull(); n = n.nextSibling())
    {
        QDomElement e = n.toElement();
        if(not e.isNull())
        {
            kDebug(/*DBG_PIGMENT*/) << e.tagName();
            if(e.tagName() == "conversion")
            {
                QDomElement eIn = e.firstChildElement("input");
                QDomElement eOut = e.firstChildElement("output");
                if(not eIn.isNull() and not eOut.isNull())
                {
                    d->conversionInfos.push_back(
                        ConversionInfo( e.attribute("function"),
                            KoID(eIn.attribute("colorModel"), ""), generateDepthID(eIn.attribute("depth"), eIn.attribute("type")),
                            KoID(eOut.attribute("colorModel"), ""), generateDepthID(eOut.attribute("depth"), eOut.attribute("type") )));
                } else {
                    kDebug(/*DBG_PIGMENT*/) << "Invalid conversion, missing <input> or <output> or both";
                }
            }
        }
    }
    kDebug(/*DBG_PIGMENT*/) << d->conversionInfos.size() << " convertions were found";
}

bool KoCtlColorProfile::load()
{
    QDomDocument doc;
    QFile file(fileName());
    if (not file.open(QIODevice::ReadOnly))
    {
        kDebug(/*DBG_PIGMENT*/) << "Can't open file : " << fileName();
        return false;
    }
    QString errorMsg;
    int errorLine;
    if (not doc.setContent(&file, &errorMsg, &errorLine)) {
        kDebug(/*DBG_PIGMENT*/) << "Can't parse file : " << fileName() << " Error at line " << errorLine << " " << errorMsg;
        file.close();
        return false;
    }
    file.close();
    QDomElement docElem = doc.documentElement();
    if(docElem.tagName() != "ctlprofile")
    {
        kDebug(/*DBG_PIGMENT*/) << "Not a ctlprofile, root tag was : " << docElem.tagName();
        return false;
    }
    QDomNode n = docElem.firstChild();
    while(not n.isNull()) {
        QDomElement e = n.toElement();
        if(not e.isNull()) {
            kDebug(/*DBG_PIGMENT*/) << e.tagName();
            if( e.tagName() == "info")
            {
                setName(e.attribute("name"));
                d->colorDepthID = generateDepthID(e.attribute("depth"), e.attribute("type"));
                d->colorModelID = KoID(e.attribute("colorModel"),"");
                kDebug(/*DBG_PIGMENT*/) << "colorModel = " << e.attribute("colorModel");;
            } else if(e.tagName() == "program")
            {
                QDomNode nCDATA = e.firstChild();
                if( not nCDATA.isNull())
                {
                    QDomCDATASection CDATA = nCDATA.toCDATASection();
                    kDebug(/*DBG_PIGMENT*/) << CDATA.data();
                    d->module = new OpenCTL::Module(name().toAscii().data());
                    d->module->setSource( CDATA.data().toAscii().data());
                    d->module->compile();
                }
            } else if(e.tagName() == "transformations")
            {
                decodeTransformations(e);
            }
        }
        n = n.nextSibling();
    }
    return true;
}
