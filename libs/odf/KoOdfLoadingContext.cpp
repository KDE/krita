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
};

KoOdfLoadingContext::KoOdfLoadingContext(KoOdfStylesReader& stylesReader, KoStore* store, const KComponentData & componentData)
        : m_store(store)
        , m_stylesReader(stylesReader)
        , m_metaXmlParsed(false)
        , m_useStylesAutoStyles(false)
        , d(0)
{
    // Ideally this should be done by KoDocument and passed as argument here...
    KoOdfReadStore oasisStore(store);
    QString dummy;
    (void)oasisStore.loadAndParse("tar:/META-INF/manifest.xml", m_manifestDoc, dummy);

    if (componentData.isValid()) {
        QString fileName( KStandardDirs::locate( "styles", "defaultstyles.xml", componentData ) );
        if ( ! fileName.isEmpty() ) {
            QFile file( fileName );
            QString errorMessage;
            if ( KoOdfReadStore::loadAndParse( &file, m_doc, errorMessage, fileName ) ) {
                m_defaultStylesReader.createStyleMap( m_doc, true );
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
    KoOdfReadStore oasisStore(m_store);
    QString dummy;
    (void)oasisStore.loadAndParse(fileName, m_manifestDoc, dummy);
}

void KoOdfLoadingContext::fillStyleStack(const KoXmlElement& object, const char* nsURI, const char* attrName, const char* family)
{
    // find all styles associated with an object and push them on the stack
    if (object.hasAttributeNS(nsURI, attrName)) {
        const QString styleName = object.attributeNS(nsURI, attrName, QString());
        const KoXmlElement * style = m_stylesReader.findStyle(styleName, family, m_useStylesAutoStyles);

        if (style)
            addStyles(style, family, m_useStylesAutoStyles);
        else
            kWarning(32500) << "style" << styleName << "not found in" << (m_useStylesAutoStyles ? "styles.xml" : "content.xml");
    }
}

void KoOdfLoadingContext::addStyles(const KoXmlElement* style, const char* family, bool usingStylesAutoStyles)
{
    Q_ASSERT(style);
    if (!style) return;

    // this recursive function is necessary as parent styles can have parents themselves
    if (style->hasAttributeNS(KoXmlNS::style, "parent-style-name")) {
        const QString parentStyleName = style->attributeNS(KoXmlNS::style, "parent-style-name", QString());
        const KoXmlElement* parentStyle = m_stylesReader.findStyle(parentStyleName, family, usingStylesAutoStyles);

        if (parentStyle)
            addStyles(parentStyle, family, usingStylesAutoStyles);
        else {
            kWarning(32500) << "Parent style not found: " << family << parentStyleName << usingStylesAutoStyles;
            //we are handling a non compliant odf file. let's at the very least load the application default, and the eventual odf default
            if (family) {
                const KoXmlElement* def = m_stylesReader.defaultStyle(family);
                if (def) {   // then, the default style for this family
                    m_styleStack.push(*def);
                }
            }
        }
    } else if (family) {
        const KoXmlElement* def = m_stylesReader.defaultStyle(family);
        if (def) {   // then, the default style for this family
            m_styleStack.push(*def);
        }
    }

    //kDebug(32500) <<"pushing style" << style->attributeNS( KoXmlNS::style,"name", QString() );
    m_styleStack.push(*style);
}

QString KoOdfLoadingContext::generator() const
{
    parseMeta();
    return m_generator;
}

void KoOdfLoadingContext::parseMeta() const
{
    if (!m_metaXmlParsed && m_store) {
        // Regardless of whether we cd into the parent directory
        // or not to find a meta.xml, restore the directory that
        // we were in afterwards.
        m_store->pushDirectory();

        // Some embedded documents to not contain their own meta.xml
        // Use the parent directory's instead.
        if (!m_store->hasFile("meta.xml"))
            // Only has an effect if there is a parent directory
            m_store->leaveDirectory();

        if (m_store->hasFile("meta.xml")) {
            KoXmlDocument metaDoc;
            KoOdfReadStore oasisStore(m_store);
            QString errorMsg;
            if (oasisStore.loadAndParse("meta.xml", metaDoc, errorMsg)) {
                KoXmlNode meta   = KoXml::namedItemNS(metaDoc, KoXmlNS::office, "document-meta");
                KoXmlNode office = KoXml::namedItemNS(meta, KoXmlNS::office, "meta");
                KoXmlElement generator = KoXml::namedItemNS(office, KoXmlNS::meta, "generator");
                if (!generator.isNull())
                    m_generator = generator.text();
            }
        }
        m_metaXmlParsed = true;

        m_store->popDirectory();
    }
}
