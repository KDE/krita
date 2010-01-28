/*
 *  Copyright (c) 2008,2010 Cyrille Berger <cberger@cberger.net>
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

#include <GTLCore/Version.h>

#if GTL_CORE_VERSION_MAJOR == 0 && GTL_CORE_VERSION_MINOR == 9 && GTL_CORE_VERSION_REVISION > 12
#include <GTLCore/CompilationMessages.h>
#endif

#include <GTLCore/PixelDescription.h>
#include <GTLCore/String.h>
#include <GTLCore/Type.h>
#include <GTLCore/Value.h>
#include <OpenCTL/Program.h>
#include <OpenCTL/Module.h>
#include "KoCtlMutex.h"
#include "KoCtlParser.h"
#include "KoCtlUtils.h"

#include "kis_debug.h"

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
    QString profileSource;
    bool loadFromSource(KoCtlColorProfile* self);
};

KoCtlColorProfile::KoCtlColorProfile() : KoColorProfile(), d(new Private)
{
    d->module = 0;
    d->middleGrayScaleFactor = 0.0883883;
    d->exposure = pow(2, 2.47393) * d->middleGrayScaleFactor;
}

KoCtlColorProfile* KoCtlColorProfile::fromFile(QString fileName)
{
    KoCtlColorProfile* profile = new KoCtlColorProfile;
    profile->setFileName(fileName);
    return profile;
}

KoCtlColorProfile* KoCtlColorProfile::fromString(QString string)
{
    KoCtlColorProfile* profile = new KoCtlColorProfile;
    profile->d->profileSource = string;
    if (profile->d->loadFromSource(profile)) {
        return profile;
    } else {
        delete profile;
        return 0;
    }
}

KoCtlColorProfile::KoCtlColorProfile(const KoCtlColorProfile& rhs) : KoColorProfile(rhs), d(new Private(*rhs.d))
{

}

KoCtlColorProfile::~KoCtlColorProfile()
{
    delete d->module;
    delete d;
}

KoColorProfile* KoCtlColorProfile::clone() const
{
    return new KoCtlColorProfile(*this);
}

bool KoCtlColorProfile::valid() const
{
    dbgPigment << d->colorModelID.isNull() << " " << d->colorDepthID.isNull() << " isCompiled: " << d->module->isCompiled();
    return (d->module && d->module->isCompiled() && !d->colorModelID.isNull() && !d->colorDepthID.isNull());
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
                && (info.sourceColorDepthID == srcDepthId || (info.sourceColorDepthID == "F"
                        && (srcDepthId == Float16BitsColorDepthID.id() || srcDepthId == Float32BitsColorDepthID.id())))
                && info.destinationColorModelID == dstModelId
                && (info.destinationColorDepthID == dstDepthId || (info.destinationColorDepthID == "F"
                        && (dstDepthId == Float16BitsColorDepthID.id() || dstDepthId == Float32BitsColorDepthID.id())))) {
            GTLCore::PixelDescription srcPixelDescription = createPixelDescription(_srcCs);
            GTLCore::PixelDescription dstPixelDescription = createPixelDescription(_dstCs);
            return new OpenCTL::Program(info.function.toAscii().data(), d->module, srcPixelDescription, dstPixelDescription);
        }
    }
    return 0;
}

QList<KoColorConversionTransformationFactory*> KoCtlColorProfile::createColorConversionTransformationFactories() const
{
    dbgPlugins << "createColorConversionTransformationFactories() " << d->conversionInfos.size();
    QList<KoColorConversionTransformationFactory*> factories;
    foreach(ConversionInfo info, d->conversionInfos) {
        dbgPlugins << info.destinationColorModelID << " " << info.destinationColorDepthID << " " << info.destinationProfile << " " << info.sourceColorModelID << " " << info.sourceColorDepthID << " " << info.sourceProfile << " " << info.function;
        if (info.sourceColorDepthID == "F" && info.destinationColorDepthID == "F") {
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
        return ctlp->name() == name() && ctlp->d->colorModelIDNumber == d->colorModelIDNumber && ctlp->d->colorDepthIDNumber == d->colorDepthIDNumber;
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
    dbgPlugins << "decodeTransformations " << elt.tagName();
    for (QDomNode nt = elt.firstChild(); !nt.isNull(); nt = nt.nextSibling()) {
        QDomElement et = nt.toElement();
        if (!et.isNull()) {
            dbgPigment << et.tagName();
            if (et.tagName() == "conversions") {
                decodeConversions(et);
            }
        }
    }
}

void KoCtlColorProfile::decodeConversions(QDomElement& elt)
{
    dbgPlugins << "decodeConversions " << elt.tagName() << " " << elt.childNodes().count();
    for (QDomNode n = elt.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            dbgPigment << e.tagName();
            if (e.tagName() == "conversion") {
                QDomElement eIn = e.firstChildElement("input");
                QDomElement eOut = e.firstChildElement("output");
                if (!eIn.isNull() && !eOut.isNull()) {
                    ConversionInfo ci;
                    ci.function = e.attribute("function");
                    ci.sourceColorModelID = eIn.attribute("colorModel");
                    ci.sourceColorDepthID = KoCtlParser::generateDepthID(eIn.attribute("depth"), eIn.attribute("type")).id();
                    ci.sourceProfile = eIn.attribute("profile");
                    ci.destinationColorModelID = eOut.attribute("colorModel");
                    ci.destinationColorDepthID = KoCtlParser::generateDepthID(eOut.attribute("depth"), eOut.attribute("type")).id();
                    ci.destinationProfile = eOut.attribute("profile");
                    if (ci.sourceColorModelID == colorModel() && ci.sourceColorDepthID == colorDepth() && ci.sourceProfile.isEmpty()) {
                        ci.sourceProfile = name();
                        d->conversionInfos.push_back(ci);
                    } else if (ci.destinationColorModelID == colorModel() && ci.destinationColorDepthID == colorDepth() && ci.destinationProfile.isEmpty()) {
                        ci.destinationProfile = name();
                        d->conversionInfos.push_back(ci);
                    } else {
                        Q_ASSERT(ci.destinationProfile == name() || ci.sourceProfile == name());
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
    QFile file(fileName());
    if (!file.open(QIODevice::ReadOnly)) {
        dbgPigment << "Can't open file : " << fileName();
        return false;
    }
    d->profileSource = file.readAll();
    file.close();
    return d->loadFromSource(this);
}

bool KoCtlColorProfile::Private::loadFromSource(KoCtlColorProfile* self)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    if (!doc.setContent(profileSource, &errorMsg, &errorLine)) {
        dbgPigment << "Can't parse profile : " << self->fileName() << " Error at line " << errorLine << " " << errorMsg;
        return false;
    }
    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() != "ctlprofile") {
        dbgPigment << "Not a ctlprofile, root tag was : " << docElem.tagName();
        return false;
    }
    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            dbgPigment << e.tagName();
            if (e.tagName() == "info") {
                self->setName(e.attribute("name"));
                colorDepthID = KoCtlParser::generateDepthID(e.attribute("depth"), e.attribute("type")).id();
                colorDepthIDNumber = KoUniqueNumberForIdServer::instance()->numberForId(colorDepthID);
                colorModelID = e.attribute("colorModel");
                colorModelIDNumber = KoUniqueNumberForIdServer::instance()->numberForId(colorModelID);
                dbgPigment << "colorModel = " << e.attribute("colorModel");;
            } else if (e.tagName() == "program") {
                QDomNode nCDATA = e.firstChild();
                if (!nCDATA.isNull()) {
                    QMutexLocker lock(ctlMutex);
                    QDomCDATASection CDATA = nCDATA.toCDATASection();
                    dbgPigment << CDATA.data();
                    module = new OpenCTL::Module();
                    module->setSource(self->name().toAscii().data(), CDATA.data().toAscii().data());
                    module->compile();
#if GTL_CORE_VERSION_MAJOR == 0 && GTL_CORE_VERSION_MINOR == 9 && GTL_CORE_VERSION_REVISION > 12
                    if (!module->isCompiled()) {
                        dbgKrita << module->compilationMessages().toString().c_str();
                    }
#endif
                }
            } else if (e.tagName() == "transformations") {
                self->decodeTransformations(e);
            }
        }
        n = n.nextSibling();
    }
    return true;
}

bool KoCtlColorProfile::save(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        dbgPigment << "Can't open file : " << fileName;
        return false;
    }
    file.write(d->profileSource.toUtf8());
    file.close();
    return true;
}

QByteArray KoCtlColorProfile::rawData() const
{
    return d->profileSource.toUtf8();
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
