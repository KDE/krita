/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoOdfLoadingContext.h"
#include <KoOdfReadStore.h>
#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoXmlNS.h>

#include <kstandarddirs.h>

#include <kdebug.h>

class KoOdfLoadingContext::Private
{
public:
    Private(KoOdfStylesReader &sr, KoStore *s)
        : store(s),
        stylesReader(sr),
        metaXmlParsed(false),
        useStylesAutoStyles(false)
    {
    }

    KoStore *store;
    KoOdfStylesReader &stylesReader;
    KoStyleStack styleStack;

    mutable QString generator;
    mutable bool metaXmlParsed;
    bool useStylesAutoStyles;

    KoXmlDocument manifestDoc;

    KoOdfStylesReader defaultStylesReader;
    KoXmlDocument doc; // the doc needs to be kept around so it is possible to access the styles
};

KoOdfLoadingContext::KoOdfLoadingContext(KoOdfStylesReader &stylesReader, KoStore* store, const KComponentData &componentData)
        : d(new Private(stylesReader, store))
{
    // Ideally this should be done by KoDocument and passed as argument here...
    KoOdfReadStore oasisStore(store);
    QString dummy;
    (void)oasisStore.loadAndParse("tar:/META-INF/manifest.xml", d->manifestDoc, dummy);

    if (componentData.isValid()) {
        QString fileName( KStandardDirs::locate( "styles", "defaultstyles.xml", componentData ) );
        if ( ! fileName.isEmpty() ) {
            QFile file( fileName );
            QString errorMessage;
            if ( KoOdfReadStore::loadAndParse( &file, d->doc, errorMessage, fileName ) ) {
                d->defaultStylesReader.createStyleMap( d->doc, true );
            }
            else {
                kWarning(30010) << "reading of defaultstyles.xml failed:" << errorMessage;
            }
        }
        else {
            kWarning(30010) << "defaultstyles.xml not found";
        }
    }
}

KoOdfLoadingContext::~KoOdfLoadingContext()
{
    delete d;
}

void KoOdfLoadingContext::setManifestFile(const QString& fileName) {
    KoOdfReadStore oasisStore(d->store);
    QString dummy;
    (void)oasisStore.loadAndParse(fileName, d->manifestDoc, dummy);
}

void KoOdfLoadingContext::fillStyleStack(const KoXmlElement& object, const char* nsURI, const char* attrName, const char* family)
{
    // find all styles associated with an object and push them on the stack
    if (object.hasAttributeNS(nsURI, attrName)) {
        const QString styleName = object.attributeNS(nsURI, attrName, QString());
        const KoXmlElement * style = d->stylesReader.findStyle(styleName, family, d->useStylesAutoStyles);

        if (style)
            addStyles(style, family, d->useStylesAutoStyles);
        else
            kWarning(32500) << "style" << styleName << "not found in" << (d->useStylesAutoStyles ? "styles.xml" : "content.xml");
    }
}

void KoOdfLoadingContext::addStyles(const KoXmlElement* style, const char* family, bool usingStylesAutoStyles)
{
    Q_ASSERT(style);
    if (!style) return;

    // this recursive function is necessary as parent styles can have parents themselves
    if (style->hasAttributeNS(KoXmlNS::style, "parent-style-name")) {
        const QString parentStyleName = style->attributeNS(KoXmlNS::style, "parent-style-name", QString());
        const KoXmlElement* parentStyle = d->stylesReader.findStyle(parentStyleName, family, usingStylesAutoStyles);

        if (parentStyle)
            addStyles(parentStyle, family, usingStylesAutoStyles);
        else {
            kWarning(32500) << "Parent style not found: " << family << parentStyleName << usingStylesAutoStyles;
            //we are handling a non compliant odf file. let's at the very least load the application default, and the eventual odf default
            if (family) {
                const KoXmlElement* def = d->stylesReader.defaultStyle(family);
                if (def) {   // then, the default style for this family
                    d->styleStack.push(*def);
                }
            }
        }
    } else if (family) {
        const KoXmlElement* def = d->stylesReader.defaultStyle(family);
        if (def) {   // then, the default style for this family
            d->styleStack.push(*def);
        }
    }

    //kDebug(32500) <<"pushing style" << style->attributeNS( KoXmlNS::style,"name", QString() );
    d->styleStack.push(*style);
}

QString KoOdfLoadingContext::generator() const
{
    if (!d->metaXmlParsed && d->store) {
        // Regardless of whether we cd into the parent directory
        // or not to find a meta.xml, restore the directory that
        // we were in afterwards.
        d->store->pushDirectory();

        // Some embedded documents to not contain their own meta.xml
        // Use the parent directory's instead.
        if (!d->store->hasFile("meta.xml"))
            // Only has an effect if there is a parent directory
            d->store->leaveDirectory();

        if (d->store->hasFile("meta.xml")) {
            KoXmlDocument metaDoc;
            KoOdfReadStore oasisStore(d->store);
            QString errorMsg;
            if (oasisStore.loadAndParse("meta.xml", metaDoc, errorMsg)) {
                KoXmlNode meta   = KoXml::namedItemNS(metaDoc, KoXmlNS::office, "document-meta");
                KoXmlNode office = KoXml::namedItemNS(meta, KoXmlNS::office, "meta");
                KoXmlElement generator = KoXml::namedItemNS(office, KoXmlNS::meta, "generator");
                if (!generator.isNull())
                    d->generator = generator.text();
            }
        }
        d->metaXmlParsed = true;

        d->store->popDirectory();
    }
    return d->generator;
}

KoStore *KoOdfLoadingContext::store() const
{
    return d->store;
}

KoOdfStylesReader &KoOdfLoadingContext::stylesReader()
{
    return d->stylesReader;
}

/**
* Get the application default styles styleReader
*/
KoOdfStylesReader &KoOdfLoadingContext::defaultStylesReader()
{
    return d->defaultStylesReader;
}

KoStyleStack &KoOdfLoadingContext::styleStack() const
{
    return d->styleStack;
}

const KoXmlDocument &KoOdfLoadingContext::manifestDocument() const
{
    return d->manifestDoc;
}

void KoOdfLoadingContext::setUseStylesAutoStyles(bool useStylesAutoStyles)
{
    d->useStylesAutoStyles = useStylesAutoStyles;
}

bool KoOdfLoadingContext::useStylesAutoStyles() const
{
    return d->useStylesAutoStyles;
}

