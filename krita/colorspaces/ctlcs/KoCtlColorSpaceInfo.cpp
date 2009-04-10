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

struct KoCtlColorSpaceInfo::ChannelInfo::Private {
};

KoCtlColorSpaceInfo::ChannelInfo::ChannelInfo() : d(new Private) {
    
}

KoCtlColorSpaceInfo::ChannelInfo::~ChannelInfo() {
    delete d;
}

struct KoCtlColorSpaceInfo::Private {
    QString fileName;
    QString colorDepthID;
    QString colorModelId;
    QString id;
    QString name;
    QString defaultProfile;
    bool isHdr;
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

bool KoCtlColorSpaceInfo::load()
{
    QDomDocument doc;
    QFile file(fileName());
    if (not file.open(QIODevice::ReadOnly))
    {
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
    if(docElem.tagName() != "ctlcolorspace")
    {
        dbgPlugins << "Not a ctlcolorspace, root tag was : " << docElem.tagName();
        return false;
    }
    d->isHdr = false;
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(not e.isNull()) {
            dbgPlugins << e.tagName();
            if( e.tagName() == "info")
            {
                CHECK_AVAILABILITY("depth");
                CHECK_AVAILABILITY("type");
                CHECK_AVAILABILITY("depth");
                d->colorDepthID = KoCtlParser::generateDepthID(e.attribute("depth"), e.attribute("type"));
                FILL_MEMBER("colorModel", colorModelId);
                FILL_MEMBER("name", name);
                FILL_MEMBER("id", id);
            } else if( e.tagName() == "defaultProfile" ) {
                FILL_MEMBER("name", defaultProfile);
            } else if( e.tagName() == "isHdr" ) {
                d->isHdr = true;
            }
        }
        n = n.nextSibling();
    }
    return true;
}

const QString& KoCtlColorSpaceInfo::colorDepthId() const
{
    return d->colorDepthID;
}

const QString& KoCtlColorSpaceInfo::colorModelId() const
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
