/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2010 Inge Wallin <inge@lysator.liu.se>

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

// Own
#include "KoOdfLoadingContext.h"

// KDE
#include <kstandarddirs.h>
#include <kdebug.h>
#include <KMimeType>

// Calligra
#include <KoOdfReadStore.h>
#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlNS.h>
#include <KoOdfManifestEntry.h>



class KoOdfLoadingContext::Private
{
public:
    Private(KoOdfStylesReader &sr, KoStore *s)
        : store(s),
        stylesReader(sr),
        generatorType(KoOdfLoadingContext::Unknown),
        metaXmlParsed(false),
        useStylesAutoStyles(false)
    {
    }

    ~Private() {
        qDeleteAll(manifestEntries);
    }

    KoStore *store;
    KoOdfStylesReader &stylesReader;
    KoStyleStack styleStack;

    mutable QString generator;
    GeneratorType generatorType;
    mutable bool metaXmlParsed;
    bool useStylesAutoStyles;

    KoXmlDocument manifestDoc;
    QHash<QString, KoOdfManifestEntry *> manifestEntries;


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

    if (!parseManifest(d->manifestDoc)) {
        kWarning(30010) << "could not parse manifest document";
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
    if (!parseManifest(d->manifestDoc)) {
        kWarning(30010) << "could not parse manifest document";
    }
}

void KoOdfLoadingContext::fillStyleStack(const KoXmlElement& object, const QString &nsURI, const QString &attrName, const QString &family)
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

void KoOdfLoadingContext::addStyles(const KoXmlElement* style, const QString &family, bool usingStylesAutoStyles)
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
            if (!family.isEmpty()) {
                const KoXmlElement* def = d->stylesReader.defaultStyle(family);
                if (def) {   // then, the default style for this family
                    d->styleStack.push(*def);
                }
            }
        }
    } else if (!family.isEmpty()) {
        const KoXmlElement* def = d->stylesReader.defaultStyle(family);
        if (def) {   // then, the default style for this family
            d->styleStack.push(*def);
        }
    }

    //kDebug(32500) <<"pushing style" << style->attributeNS( KoXmlNS::style,"name", QString() );
    d->styleStack.push(*style);
}

void KoOdfLoadingContext::parseGenerator() const
{
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
            if (!generator.isNull()) {
                d->generator = generator.text();
                if (d->generator.startsWith("Calligra")) {
                    d->generatorType = Calligra;
                }
                // NeoOffice is a port of OpenOffice to Mac OS X
                else if (d->generator.startsWith("OpenOffice.org") || d->generator.startsWith("NeoOffice") ||
                         d->generator.startsWith("LibreOffice") || d->generator.startsWith("StarOffice") ||
                         d->generator.startsWith("Lotus Symphony")) {
                    d->generatorType = OpenOffice;
                }
                else if (d->generator.startsWith("MicrosoftOffice")) {
                    d->generatorType = MicrosoftOffice;
                }
            }
        }
    }
    d->metaXmlParsed = true;

    d->store->popDirectory();
}

QString KoOdfLoadingContext::generator() const
{
    if (!d->metaXmlParsed && d->store) {
        parseGenerator();
    }
    return d->generator;
}

KoOdfLoadingContext::GeneratorType KoOdfLoadingContext::generatorType() const
{
    if (!d->metaXmlParsed && d->store) {
        parseGenerator();
    }
    return d->generatorType;
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

void KoOdfLoadingContext::setUseStylesAutoStyles(bool useStylesAutoStyles)
{
    d->useStylesAutoStyles = useStylesAutoStyles;
}

bool KoOdfLoadingContext::useStylesAutoStyles() const
{
    return d->useStylesAutoStyles;
}

QString KoOdfLoadingContext::mimeTypeForPath(const QString& path, bool guess) const
{
    QHash<QString, KoOdfManifestEntry *>::iterator it(d->manifestEntries.find(path));
    if (it == d->manifestEntries.end()) {
        // try to find it with an added / at the end
        QString dirPath = path + '/';
        it = d->manifestEntries.find(dirPath);
    }
    if (it != d->manifestEntries.end()) {
        QString mimeType = it.value()->mediaType();

        // figure out mimetype by content if it is not provided
        if (mimeType.isEmpty() && guess) {
            Q_ASSERT(!d->store->isOpen());
            if (d->store->open(path)) {
                KoStoreDevice device(d->store);
                QByteArray data = device.read(16384);
                d->store->close();
                KMimeType::Ptr mtp = KMimeType::findByContent(data);
                mimeType = mtp->name();
                if (!mimeType.isEmpty()) {
                    it.value()->setMediaType(mimeType);
                }
            }
        }

        return mimeType;
    }
    else {
        return QString();
    }
}

QList<KoOdfManifestEntry*> KoOdfLoadingContext::manifestEntries() const
{
    return d->manifestEntries.values();
}

bool KoOdfLoadingContext::parseManifest(const KoXmlDocument &manifestDocument)
{
    // First find the manifest:manifest node.
    KoXmlNode  n = manifestDocument.firstChild();
    kDebug(30006) << "Searching for manifest:manifest " << n.toElement().nodeName();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement()) {
            kDebug(30006) << "NOT element";
            continue;
        } else {
            kDebug(30006) << "element";
        }

        kDebug(30006) << "name:" << n.toElement().localName()
                      << "namespace:" << n.toElement().namespaceURI();

        if (n.toElement().localName() == "manifest"
            && n.toElement().namespaceURI() == KoXmlNS::manifest)
        {
            kDebug(30006) << "found manifest:manifest";
            break;
        }
    }
    if (n.isNull()) {
        kDebug(30006) << "Could not find manifest:manifest";
        return false;
    }

    // Now loop through the children of the manifest:manifest and
    // store all the manifest:file-entry elements.
    const KoXmlElement  manifestElement = n.toElement();
    for (n = manifestElement.firstChild(); !n.isNull(); n = n.nextSibling()) {

        if (!n.isElement())
            continue;

        KoXmlElement el = n.toElement();
        if (!(el.localName() == "file-entry" && el.namespaceURI() == KoXmlNS::manifest))
            continue;

        QString fullPath  = el.attributeNS(KoXmlNS::manifest, "full-path", QString());
        QString mediaType = el.attributeNS(KoXmlNS::manifest, "media-type", QString(""));
        QString version   = el.attributeNS(KoXmlNS::manifest, "version", QString());

        // Only if fullPath is valid, should we store this entry.
        // If not, we don't bother to find out exactly what is wrong, we just skip it.
        if (!fullPath.isNull()) {
            d->manifestEntries.insert(fullPath,
                                      new KoOdfManifestEntry(fullPath, mediaType, version));
        }
    }

    return true;
}
