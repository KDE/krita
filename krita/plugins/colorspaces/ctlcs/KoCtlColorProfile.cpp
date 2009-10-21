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

#include <math.h>

#include "DebugPigment.h"

#include <QDomDocument>
#include <QFile>
#include <QMutexLocker>

#include <QString>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <KoCtlColorConversionTransformation.h>
#include <KoUniqueNumberForIdServer.h>

#include <GTLCore/PixelDescription.h>
#include <GTLCore/String.h>
#include <GTLCore/Type.h>
#include <GTLCore/Value.h>
#include <OpenCTL/Program.h>
#include <OpenCTL/Module.h>
#include "KoCtlMutex.h"
#include "KoCtlParser.h"

struct ConversionInfo {
    QString sourceColorModelID;
    QString sourceColorDepthID;
    QString sourceProfile;
    QString destinationColorModelID;
    QString destinationColorDepthID;
    QString destinationProfile;
    QString function;
};

struct KoCtlColorProfile::Private {
    OpenCTL::Module* module;
    QList<ConversionInfo> conversionInfos;
    QString colorModelID;
    quint32 colorModelIDNumber;
    QString colorDepthID;
    quint32 colorDepthIDNumber;
    qreal exposure;
    qreal middleGrayScaleFactor;
};

KoCtlColorProfile::KoCtlColorProfile(QString filename) : KoColorProfile(filename), d(new Private)
{
    d->module = 0;
    d->middleGrayScaleFactor = 0.0883883;
    d->exposure = pow(2, 2.47393) * d->middleGrayScaleFactor;
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
    dbgPigment << d->colorModelID.isNull() << " " << d->colorDepthID.isNull();
    return (d->module and d->module->isCompiled()
            and not d->colorModelID.isNull() and not d->colorDepthID.isNull());
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

OpenCTL::Program* KoCtlColorProfile::createColorConversionProgram(const KoColorSpace* _srcCs, const KoColorSpace* _dstCs) const
{
    QString srcModelId = _srcCs->colorModelId().id();
    QString srcDepthId = _srcCs->colorDepthId().id();
    QString dstModelId = _dstCs->colorModelId().id();
    QString dstDepthId = _dstCs->colorDepthId().id();
    foreach(ConversionInfo info, d->conversionInfos) {
        if (info.sourceColorModelID == srcModelId
                and(info.sourceColorDepthID == srcDepthId or(info.sourceColorDepthID == "F"
                        and(srcDepthId == Float16BitsColorDepthID.id() or srcDepthId == Float32BitsColorDepthID.id())))
                and info.destinationColorModelID == dstModelId
                and(info.destinationColorDepthID == dstDepthId or(info.destinationColorDepthID == "F"
                        and(dstDepthId == Float16BitsColorDepthID.id() or dstDepthId == Float32BitsColorDepthID.id())))) {
            GTLCore::PixelDescription srcPixelDescription = createPixelDescription(_srcCs);
            GTLCore::PixelDescription dstPixelDescription = createPixelDescription(_dstCs);
            return new OpenCTL::Program(info.function.toAscii().data(), d->module, srcPixelDescription, dstPixelDescription);
        }
    }
    return 0;
}

QList<KoColorConversionTransformationFactory*> KoCtlColorProfile::createColorConversionTransformationFactories() const
{
    QList<KoColorConversionTransformationFactory*> factories;
    foreach(ConversionInfo info, d->conversionInfos) {
        if (info.sourceColorDepthID == "F" and info.destinationColorDepthID == "F") {
            factories.push_back(
                new KoCtlColorConversionTransformationFactory(
                    info.sourceColorModelID, Float16BitsColorDepthID.id(), info.sourceProfile,
                    info.destinationColorModelID, Float16BitsColorDepthID.id(), info.destinationProfile));
            factories.push_back(
                new KoCtlColorConversionTransformationFactory(
                    info.sourceColorModelID, Float32BitsColorDepthID.id(), info.sourceProfile,
                    info.destinationColorModelID, Float32BitsColorDepthID.id(), info.destinationProfile));
        } else if (info.sourceColorDepthID == "F") {
            factories.push_back(
                new KoCtlColorConversionTransformationFactory(
                    info.sourceColorModelID, Float16BitsColorDepthID.id(),
                    info.sourceProfile, info.destinationColorModelID, info.destinationColorDepthID,
                    info.destinationProfile));
            factories.push_back(
                new KoCtlColorConversionTransformationFactory(
                    info.sourceColorModelID, Float32BitsColorDepthID.id(),
                    info.sourceProfile, info.destinationColorModelID, info.destinationColorDepthID,
                    info.destinationProfile));
        } else if (info.destinationColorDepthID == "F") {
            factories.push_back(
                new KoCtlColorConversionTransformationFactory(
                    info.sourceColorModelID, info.sourceColorDepthID,
                    info.sourceProfile, info.destinationColorModelID, Float16BitsColorDepthID.id(),
                    info.destinationProfile));
            factories.push_back(
                new KoCtlColorConversionTransformationFactory(
                    info.sourceColorModelID, info.sourceColorDepthID,
                    info.sourceProfile, info.destinationColorModelID, Float32BitsColorDepthID.id(),
                    info.destinationProfile));
        } else {
            factories.push_back(
                new KoCtlColorConversionTransformationFactory(
                    info.sourceColorModelID, info.sourceColorDepthID,
                    info.sourceProfile, info.destinationColorModelID, info.destinationColorDepthID,
                    info.destinationProfile));
        }
    }
    return factories;
}

bool KoCtlColorProfile::operator==(const KoColorProfile& p) const
{
    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(&p);
    if (ctlp) {
        return ctlp->name() == name() and ctlp->d->colorModelIDNumber == d->colorModelIDNumber and ctlp->d->colorDepthIDNumber == d->colorDepthIDNumber;
    } else {
        return false;
    }
}

QString KoCtlColorProfile::colorModel() const
{
    return d->colorModelID;
}

QString KoCtlColorProfile::colorDepth() const
{
    return d->colorDepthID;
}

void KoCtlColorProfile::decodeTransformations(QDomElement& elt)
{
    for (QDomNode nt = elt.firstChild(); not nt.isNull(); nt = nt.nextSibling()) {
        QDomElement et = nt.toElement();
        if (not et.isNull()) {
            dbgPigment << et.tagName();
            if (et.tagName() == "conversions") {
                decodeConversions(et);
            }
        }
    }
}

void KoCtlColorProfile::decodeConversions(QDomElement& elt)
{
    for (QDomNode n = elt.firstChild(); not n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (not e.isNull()) {
            dbgPigment << e.tagName();
            if (e.tagName() == "conversion") {
                QDomElement eIn = e.firstChildElement("input");
                QDomElement eOut = e.firstChildElement("output");
                if (not eIn.isNull() and not eOut.isNull()) {
                    ConversionInfo ci;
                    ci.function = e.attribute("function");
                    ci.sourceColorModelID = eIn.attribute("colorModel");
                    ci.sourceColorDepthID = KoCtlParser::generateDepthID(eIn.attribute("depth"), eIn.attribute("type")).id();
                    ci.sourceProfile = eIn.attribute("profile");
                    ci.destinationColorModelID = eOut.attribute("colorModel");
                    ci.destinationColorDepthID = KoCtlParser::generateDepthID(eOut.attribute("depth"), eOut.attribute("type")).id();
                    ci.destinationProfile = eOut.attribute("profile");
                    if (ci.sourceColorModelID == colorModel() and ci.sourceColorDepthID == colorDepth() and ci.sourceProfile.isEmpty()) {
                        ci.sourceProfile = name();
                        d->conversionInfos.push_back(ci);
                    } else if (ci.destinationColorModelID == colorModel() and ci.destinationColorDepthID == colorDepth() and ci.destinationProfile.isEmpty()) {
                        ci.destinationProfile = name();
                        d->conversionInfos.push_back(ci);
                    } else {
                        Q_ASSERT(ci.destinationProfile == name() or ci.sourceProfile == name());
                        d->conversionInfos.push_back(ci);
                    }
                } else {
                    dbgPigment << "Invalid conversion, missing <input> or <output> or both";
                }
            }
        }
    }
    dbgPigment << d->conversionInfos.size() << " convertions were found";
}

bool KoCtlColorProfile::load()
{
    QDomDocument doc;
    QFile file(fileName());
    if (not file.open(QIODevice::ReadOnly)) {
        dbgPigment << "Can't open file : " << fileName();
        return false;
    }
    QString errorMsg;
    int errorLine;
    if (not doc.setContent(&file, &errorMsg, &errorLine)) {
        dbgPigment << "Can't parse file : " << fileName() << " Error at line " << errorLine << " " << errorMsg;
        file.close();
        return false;
    }
    file.close();
    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() != "ctlprofile") {
        dbgPigment << "Not a ctlprofile, root tag was : " << docElem.tagName();
        return false;
    }
    QDomNode n = docElem.firstChild();
    while (not n.isNull()) {
        QDomElement e = n.toElement();
        if (not e.isNull()) {
            dbgPigment << e.tagName();
            if (e.tagName() == "info") {
                setName(e.attribute("name"));
                d->colorDepthID = KoCtlParser::generateDepthID(e.attribute("depth"), e.attribute("type")).id();
                d->colorDepthIDNumber = KoUniqueNumberForIdServer::instance()->numberForId(d->colorDepthID);
                d->colorModelID = e.attribute("colorModel");
                d->colorModelIDNumber = KoUniqueNumberForIdServer::instance()->numberForId(d->colorModelID);
                dbgPigment << "colorModel = " << e.attribute("colorModel");;
            } else if (e.tagName() == "program") {
                QDomNode nCDATA = e.firstChild();
                if (not nCDATA.isNull()) {
                    QMutexLocker lock(ctlMutex);
                    QDomCDATASection CDATA = nCDATA.toCDATASection();
                    dbgPigment << CDATA.data();
                    d->module = new OpenCTL::Module(name().toAscii().data());
                    d->module->setSource(CDATA.data().toAscii().data());
                    d->module->compile();
                }
            } else if (e.tagName() == "transformations") {
                decodeTransformations(e);
            }
        }
        n = n.nextSibling();
    }
    return true;
}

GTLCore::PixelDescription KoCtlColorProfile::createPixelDescription(const KoColorSpace* cs) const
{
    QList<KoChannelInfo*> channels = cs->channels();
    std::vector<const GTLCore::Type* > types;
    foreach(KoChannelInfo* info, channels) {
        switch (info->channelValueType()) {
        case KoChannelInfo::UINT8:
            types.push_back(GTLCore::Type::UnsignedInteger8);
            break;
        case KoChannelInfo::UINT16:
            types.push_back(GTLCore::Type::UnsignedInteger16);
            break;
        case KoChannelInfo::UINT32:
            types.push_back(GTLCore::Type::UnsignedInteger32);
            break;
        case KoChannelInfo::FLOAT16:
            types.push_back(GTLCore::Type::Half);
            break;
        case KoChannelInfo::FLOAT32:
            types.push_back(GTLCore::Type::Float);
            break;
        case KoChannelInfo::OTHER:
        case KoChannelInfo::FLOAT64:
            Q_ASSERT(false);
            break;
        case KoChannelInfo::INT8:
            types.push_back(GTLCore::Type::Integer8);
            break;
        case KoChannelInfo::INT16:
            types.push_back(GTLCore::Type::Integer16);
            break;
        }
    }
    Q_ASSERT(types.size() == cs->channelCount());
    return GTLCore::PixelDescription(types);
}

QVariant KoCtlColorProfile::property(const QString& _name) const
{
    if (_name == "exposure") {
        return d->exposure;
    } else {
        dbgPigment << "Not CTL property " << _name;
        return KoColorProfile::property(_name);
    }
}

void KoCtlColorProfile::setProperty(const QString& _name, const QVariant& _variant)
{
    if (_name == "exposure") {
        d->exposure = pow(2, _variant.toDouble() + 2.47393) * d->middleGrayScaleFactor;
    } else {
        dbgPigment << "Not CTL property " << _name;
        return KoColorProfile::setProperty(_name, _variant);
    }
}
