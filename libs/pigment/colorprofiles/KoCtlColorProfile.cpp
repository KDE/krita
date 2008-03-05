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

#include <OpenCTL/Module.h>

#define and &&
#define or ||
#define not !

struct KoCtlColorProfile::Private {
    OpenCTL::Module* module;
};

KoCtlColorProfile::KoCtlColorProfile(QString filename) : KoColorProfile(filename), d(new Private)
{
    d->module = 0;
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
    return (d->module and d->module->isCompiled());
}

bool KoCtlColorProfile::isSuitableForOutput() const
{
}

bool KoCtlColorProfile::isSuitableForPrinting() const
{
    
}

bool KoCtlColorProfile::isSuitableForDisplay() const
{
}

bool KoCtlColorProfile::operator==(const KoColorProfile&) const
{
    return false;
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
         kDebug(/*DBG_PIGMENT*/) << e.tagName() << endl; // the node really is an element.
         if( e.tagName() == "info")
         {
             setName(e.attribute("name"));
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
         }
     }
     n = n.nextSibling();
 }
}
