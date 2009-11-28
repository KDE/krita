
/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "KoCtlColorSpaceInfo.h"
#include <QFile>
#include <QDomDocument>
#include <kis_debug.h>
#include "KoCtlParser.h"
#include <KoID.h>
#include <GTLCore/PixelDescription.h>
#include <GTLCore/Type.h>
#include <klocale.h>

#include "KoCtlAccumulator.h"

struct KoCtlColorSpaceInfo::ChannelInfo::Private {
    Private() : color(0, 0, 0) {}
    QString name;
    QString shortName;
    qint32 position;
    qint32 index;
    enum KoChannelInfo::enumChannelType channelType;
    enum KoChannelInfo::enumChannelValueType valueType;
    qint32 size;
    QColor color;
};

KoCtlColorSpaceInfo::ChannelInfo::ChannelInfo() : d(new Private)
{
}

KoCtlColorSpaceInfo::ChannelInfo::~ChannelInfo()
{
    delete d;
}

const QString& KoCtlColorSpaceInfo::ChannelInfo::name() const
{
    return d->name;
}

const QString& KoCtlColorSpaceInfo::ChannelInfo::shortName() const
{
    return d->shortName;
}

qint32 KoCtlColorSpaceInfo::ChannelInfo::position() const
{
    return d->position;
}

qint32 KoCtlColorSpaceInfo::ChannelInfo::index() const
{
    return d->index;
}

enum KoChannelInfo::enumChannelType KoCtlColorSpaceInfo::ChannelInfo::channelType() const
{
    return d->channelType;
}

enum KoChannelInfo::enumChannelValueType KoCtlColorSpaceInfo::ChannelInfo::valueType() const
{
    return d->valueType;
}

qint32 KoCtlColorSpaceInfo::ChannelInfo::size() const
{
    return d->size;
}

const QColor& KoCtlColorSpaceInfo::ChannelInfo::color() const
{
    return d->color;
}

struct KoCtlColorSpaceInfo::Private {
    Private() : pixelDescription(0) {}
    ~Private() {
        delete pixelDescription;
    }
    QString fileName;
    KoID colorDepthID;
    KoID colorModelId;
    QString id;
    QString name;
    QString defaultProfile;
    bool isHdr;
    qint32 referenceDepth;
    quint32 colorChannelCount;
    QList<const KoCtlColorSpaceInfo::ChannelInfo*> channels;
    quint32 pixelSize;
    GTLCore::PixelDescription* pixelDescription;
    int alphaPos;
};

KoCtlColorSpaceInfo::KoCtlColorSpaceInfo(const QString& _xmlfile) : d(new Private)
{
    d->fileName = _xmlfile;
}

KoCtlColorSpaceInfo::~KoCtlColorSpaceInfo()
{
    delete d;
}

const QString& KoCtlColorSpaceInfo::fileName() const
{
    return d->fileName;
}

#define CHECK_AVAILABILITY(attribute) \
    if(!e.hasAttribute(attribute)) { \
        dbgPlugins << "Missing: " << attribute; \
        return false; \
    }

#define FILL_MEMBER(attributeName, member) \
    CHECK_AVAILABILITY(attributeName) \
    d->member = e.attribute(attributeName);

#define FILL_CI_MEMBER(attributeName, member) \
    CHECK_AVAILABILITY(attributeName) \
    info->d->member = e.attribute(attributeName);

#define FILL_CI_MEMBER_I18N(attributeName, member) \
    CHECK_AVAILABILITY(attributeName) \
    info->d->member = i18n(e.attribute(attributeName).toUtf8().data());

bool KoCtlColorSpaceInfo::load()
{
    QDomDocument doc;
    QFile file(fileName());
    dbgPlugins << "Loading cs: " << fileName();
    if (not file.open(QIODevice::ReadOnly)) {
        dbgPlugins << "Can't open file : " << fileName();
        return false;
    }
    QString errorMsg;
    int errorLine;
    if (not doc.setContent(&file, &errorMsg, &errorLine)) {
        dbgPlugins << "Can't parse file : " << fileName() << " Error at line " << errorLine << " " << errorMsg;
        file.close();
        return false;
    }
    file.close();
    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() != "ctlcolorspace") {
        dbgPlugins << "Not a ctlcolorspace, root tag was : " << docElem.tagName();
        return false;
    }
    d->isHdr = false;
    d->colorChannelCount = 0;
    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            dbgPlugins << e.tagName();
            if (e.tagName() == "info") {
                CHECK_AVAILABILITY("depth");
                CHECK_AVAILABILITY("type");
                CHECK_AVAILABILITY("depth");
                d->colorDepthID = KoCtlParser::generateDepthID(e.attribute("depth"), e.attribute("type"));
                d->referenceDepth = e.attribute("depth").toInt();
                CHECK_AVAILABILITY("colorModelId");
                CHECK_AVAILABILITY("colorModelName");
                d->colorModelId = KoID(e.attribute("colorModelId"), i18n(e.attribute("colorModelName").toUtf8().data()));
                FILL_MEMBER("name", name);
                FILL_MEMBER("id", id);
            } else if (e.tagName() == "defaultProfile") {
                FILL_MEMBER("name", defaultProfile);
            } else if (e.tagName() == "isHdr") {
                d->isHdr = true;
            } else if (e.tagName() == "channels") {
                std::vector<const GTLCore::Type* > channelTypes;
                d->alphaPos = -1;
                QDomNode n = e.firstChild();
                int pos = 0;
                while (!n.isNull()) {
                    QDomElement e = n.toElement();
                    if (!e.isNull()) {
                        dbgPlugins << e.tagName();
                        if (e.tagName() != "channel") return false;
                        ChannelInfo* info = new ChannelInfo;
                        FILL_CI_MEMBER_I18N("name", name);
                        FILL_CI_MEMBER("shortName", shortName);
                        CHECK_AVAILABILITY("index");
                        info->d->index = e.attribute("index").toInt();

                        // Parse channelType
                        CHECK_AVAILABILITY("channelType");
                        QString channelType = e.attribute("channelType");
                        if (channelType == "Color") {
                            ++d->colorChannelCount;
                            info->d->channelType = KoChannelInfo::COLOR;
                        } else if (channelType == "Alpha") {
                            info->d->channelType = KoChannelInfo::ALPHA;
                            d->alphaPos = pos;
                        } else {
                            dbgPlugins << "Invalid channel type: " << channelType;
                            return false;
                        }

                        // Parse valueType
                        CHECK_AVAILABILITY("valueType");
                        QString valueType = e.attribute("valueType");
                        if (valueType == "float32") {
                            info->d->valueType = KoChannelInfo::FLOAT32;
                            channelTypes.push_back(GTLCore::Type::Float32);
                        } else if (valueType == "float16") {
                            info->d->valueType = KoChannelInfo::FLOAT16;
                            channelTypes.push_back(GTLCore::Type::Float16);
                        } else if (valueType == "uint8") {
                            info->d->valueType = KoChannelInfo::UINT8;
                            channelTypes.push_back(GTLCore::Type::UnsignedInteger8);
                        } else if (valueType == "uint16") {
                            info->d->valueType = KoChannelInfo::UINT16;
                            channelTypes.push_back(GTLCore::Type::UnsignedInteger16);
                        } else {
                            dbgPlugins << "Invalid value type: " << valueType;
                            return false;
                        }

                        // Parse size
                        CHECK_AVAILABILITY("size");
                        info->d->size = e.attribute("size").toInt();

                        // Parse color
                        if (e.hasAttribute("color")) {
                            QStringList colorlist = e.attribute("color").split(',');
                            if (colorlist.size() != 3) return false;
                            info->d->color = QColor(colorlist[0].toInt(), colorlist[1].toInt(), colorlist[2].toInt());
                        }
                        d->channels.push_back(info);
                        ++pos;
                    }
                    n = n.nextSibling();
                }
                d->pixelDescription = new GTLCore::PixelDescription(channelTypes, d->alphaPos);
            }
        }
        n = n.nextSibling();
    }
    if (d->channels.size() == 0) return false;

    int currentPos = 0;
    for (int i = 0; i < d->channels.size(); ++i) {
        int oldPos = currentPos;
        foreach(const ChannelInfo* info, d->channels) {
            if (info->index() == i) {
                const_cast<ChannelInfo*>(info)->d->position = currentPos;
                currentPos += info->size();
            }
        }
        if (currentPos == oldPos) return false;
    }
    d->pixelSize = currentPos;

    return true;
}

const KoID& KoCtlColorSpaceInfo::colorDepthId() const
{
    return d->colorDepthID;
}

const KoID& KoCtlColorSpaceInfo::colorModelId() const
{
    return d->colorModelId;
}

const QString& KoCtlColorSpaceInfo::colorSpaceId() const
{
    return d->id;
}

const QString& KoCtlColorSpaceInfo::name() const
{
    return d->name;
}

const QString& KoCtlColorSpaceInfo::defaultProfile() const
{
    return d->defaultProfile;
}

bool KoCtlColorSpaceInfo::isHdr() const
{
    return d->isHdr;
}

const QList<const KoCtlColorSpaceInfo::ChannelInfo*>& KoCtlColorSpaceInfo::channels() const
{
    return d->channels;
}

qint32 KoCtlColorSpaceInfo::referenceDepth() const
{
    return d->referenceDepth;
}

quint32 KoCtlColorSpaceInfo::colorChannelCount() const
{
    return d->colorChannelCount;
}

quint32 KoCtlColorSpaceInfo::pixelSize() const
{
    return d->pixelSize;
}

const GTLCore::PixelDescription& KoCtlColorSpaceInfo::pixelDescription() const
{
    return *d->pixelDescription;
}

int KoCtlColorSpaceInfo::alphaPos() const
{
    return d->alphaPos;
}

QList<KoCtlAccumulator*> KoCtlColorSpaceInfo::accumulators() const
{
    QList<KoCtlAccumulator*> accs;
    foreach(const KoCtlColorSpaceInfo::ChannelInfo* info, d->channels) {
        switch (info->valueType()) {
        case KoChannelInfo::UINT8:
            accs.push_back(new KoCtlAccumulatorImpl<quint8>(info->position()));
            break;
        case KoChannelInfo::UINT16:
            accs.push_back(new KoCtlAccumulatorImpl<quint16>(info->position()));
            break;
        case KoChannelInfo::FLOAT16:
            accs.push_back(new KoCtlAccumulatorImpl<half>(info->position()));
            break;
        case KoChannelInfo::FLOAT32:
            accs.push_back(new KoCtlAccumulatorImpl<float>(info->position()));
            break;
        default:
            qFatal("unsupported");
        }
    }
    return accs;
}
