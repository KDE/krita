/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOdfStylesReader.h"

#include "KoGenStyles.h"
#include "KoXmlNS.h"

#include <QtCore/QBuffer>

#include <kdebug.h>
#include <kglobal.h>

#include <KoXmlReader.h>
#include <KoOdfNotesConfiguration.h>

class KoOdfStylesReader::Private
{
public:

    Private()
    {
        globalFootnoteConfiguration.setNoteClass(KoOdfNotesConfiguration::Footnote);
        globalEndnoteConfiguration.setNoteClass(KoOdfNotesConfiguration::Endnote);
    }

    QHash < QString /*family*/, QHash < QString /*name*/, KoXmlElement* > > customStyles;
    // auto-styles in content.xml
    QHash < QString /*family*/, QHash < QString /*name*/, KoXmlElement* > > contentAutoStyles;
    // auto-styles in styles.xml
    QHash < QString /*family*/, QHash < QString /*name*/, KoXmlElement* > > stylesAutoStyles;
    QHash < QString /*family*/, KoXmlElement* > defaultStyles;

    QHash < QString /*name*/, KoXmlElement* > styles; // page-layout, font-face etc.
    QHash < QString /*name*/, KoXmlElement* > masterPages;
    QHash < QString /*name*/, KoXmlElement* > presentationPageLayouts;
    QHash < QString /*name*/, KoXmlElement* > drawStyles;

    KoXmlElement           officeStyle;
    KoXmlElement           layerSet;

    DataFormatsMap         dataFormats;

    // XXX: there can als be notes configuration objects _per_ section.
    KoOdfNotesConfiguration globalFootnoteConfiguration;
    KoOdfNotesConfiguration globalEndnoteConfiguration;
    KoOdfNotesConfiguration defaultNoteConfiguration;

    KoOdfLineNumberingConfiguration lineNumberingConfiguration;

};

KoOdfStylesReader::KoOdfStylesReader()
    : d(new Private)
{
}

KoOdfStylesReader::~KoOdfStylesReader()
{
    typedef QHash<QString, KoXmlElement*> AutoStylesMap;
    foreach(const AutoStylesMap& map, d->customStyles)
        qDeleteAll(map);
    foreach(const AutoStylesMap& map, d->contentAutoStyles)
        qDeleteAll(map);
    foreach(const AutoStylesMap& map, d->stylesAutoStyles)
        qDeleteAll(map);
    foreach(const DataFormatsMap::mapped_type& dataFormat, d->dataFormats)
        delete dataFormat.second;
    qDeleteAll(d->defaultStyles);
    qDeleteAll(d->styles);
    qDeleteAll(d->masterPages);
    qDeleteAll(d->presentationPageLayouts);
    qDeleteAll(d->drawStyles);
    delete d;
}

void KoOdfStylesReader::createStyleMap(const KoXmlDocument& doc, bool stylesDotXml)
{
    const KoXmlElement docElement  = doc.documentElement();
    // We used to have the office:version check here, but better let the apps do that
    KoXmlElement fontStyles = KoXml::namedItemNS(docElement, KoXmlNS::office, "font-face-decls");

    if (!fontStyles.isNull()) {
        //kDebug(30003) <<"Starting reading in font-face-decls...";
        insertStyles(fontStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent);
    }// else
    //   kDebug(30003) <<"No items found";

    //kDebug(30003) <<"Starting reading in office:automatic-styles. stylesDotXml=" << stylesDotXml;

    KoXmlElement autoStyles = KoXml::namedItemNS(docElement, KoXmlNS::office, "automatic-styles");
    if (!autoStyles.isNull()) {
        insertStyles(autoStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent);
    }// else
    //    kDebug(30003) <<"No items found";


    //kDebug(30003) <<"Reading in master styles";

    KoXmlNode masterStyles = KoXml::namedItemNS(docElement, KoXmlNS::office, "master-styles");
    if (!masterStyles.isNull()) {
        KoXmlElement master;
        forEachElement(master, masterStyles) {
            if (master.localName() == "master-page" &&
                master.namespaceURI() == KoXmlNS::style) {
                const QString name = master.attributeNS(KoXmlNS::style, "name", QString());
                kDebug(30003) << "Master style: '" << name << "' loaded";
                d->masterPages.insert(name, new KoXmlElement(master));
            } else if (master.localName() == "layer-set" && master.namespaceURI() == KoXmlNS::draw) {
                kDebug(30003) << "Master style: layer-set loaded";
                d->layerSet = master;
            } else
                // OASIS docu mentions style:handout-master and draw:layer-set here
                kWarning(30003) << "Unknown tag " << master.tagName() << " in office:master-styles";
        }
    }


    kDebug(30003) << "Starting reading in office:styles";

    const KoXmlElement officeStyle = KoXml::namedItemNS(docElement, KoXmlNS::office, "styles");

    if (!officeStyle.isNull()) {
        d->officeStyle = officeStyle;
        insertOfficeStyles(officeStyle);
    }

    //kDebug(30003) <<"Styles read in.";
}

QHash<QString, KoXmlElement*> KoOdfStylesReader::customStyles(const QString& family) const
{
    if (family.isNull())
        return QHash<QString, KoXmlElement*>();
    return d->customStyles.value(family);
}

QHash<QString, KoXmlElement*> KoOdfStylesReader::autoStyles(const QString& family, bool stylesDotXml) const
{
    if (family.isNull())
        return QHash<QString, KoXmlElement*>();
    return stylesDotXml ? d->stylesAutoStyles.value(family) : d->contentAutoStyles.value(family);
}

KoOdfStylesReader::DataFormatsMap KoOdfStylesReader::dataFormats() const
{
    return d->dataFormats;
}

KoOdfNotesConfiguration KoOdfStylesReader::globalNotesConfiguration(KoOdfNotesConfiguration::NoteClass noteClass) const
{
    switch (noteClass) {
    case (KoOdfNotesConfiguration::Endnote):
        return d->globalEndnoteConfiguration;
    case (KoOdfNotesConfiguration::Footnote):
        return d->globalFootnoteConfiguration;
    default:
        return d->defaultNoteConfiguration;
    }
}

KoOdfLineNumberingConfiguration KoOdfStylesReader::lineNumberingConfiguration() const
{
    return d->lineNumberingConfiguration;
}


void KoOdfStylesReader::insertOfficeStyles(const KoXmlElement& styles)
{
    KoXmlElement e;
    forEachElement(e, styles) {
        const QString localName = e.localName();
        const QString ns = e.namespaceURI();
        if ((ns == KoXmlNS::svg && (
                localName == "linearGradient"
                || localName == "radialGradient"))
            || (ns == KoXmlNS::draw && (
                    localName == "gradient"
                    || localName == "hatch"
                    || localName == "fill-image"
                    || localName == "marker"
                    || localName == "stroke-dash"
                    || localName == "opacity"))
            || (ns == KoXmlNS::koffice && (
                    localName == "conicalGradient"))
            ) {
            const QString name = e.attributeNS(KoXmlNS::draw, "name", QString());
            Q_ASSERT(!name.isEmpty());
            KoXmlElement* ep = new KoXmlElement(e);
            d->drawStyles.insert(name, ep);
        } else
            insertStyle(e, CustomInStyles);
    }
}

void KoOdfStylesReader::insertStyles(const KoXmlElement& styles, TypeAndLocation typeAndLocation)
{
    //kDebug(30003) <<"Inserting styles from" << styles.tagName();
    KoXmlElement e;
    forEachElement(e, styles)
            insertStyle(e, typeAndLocation);
}

void KoOdfStylesReader::insertStyle(const KoXmlElement& e, TypeAndLocation typeAndLocation)
{
    const QString localName = e.localName();
    const QString ns = e.namespaceURI();
    const QString name = e.attributeNS(KoXmlNS::style, "name", QString());

    if ((ns == KoXmlNS::style && localName == "style")
        || (ns == KoXmlNS::text && localName == "list-style")) {
        const QString family = localName == "list-style" ? "list" : e.attributeNS(KoXmlNS::style, "family", QString());

        if (typeAndLocation == AutomaticInContent) {
            QHash<QString, KoXmlElement*>& dict = d->contentAutoStyles[ family ];
            if (dict.contains(name)) {
                kDebug(30003) << "Auto-style: '" << name << "' already exists";
                delete dict.take(name);
            }
            dict.insert(name, new KoXmlElement(e));
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else if (typeAndLocation == AutomaticInStyles) {
            QHash<QString, KoXmlElement*>& dict = d->stylesAutoStyles[ family ];
            if (dict.contains(name)) {
                kDebug(30003) << "Auto-style: '" << name << "' already exists";
                delete dict.take(name);
            }
            dict.insert(name, new KoXmlElement(e));
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else {
            QHash<QString, KoXmlElement*>& dict = d->customStyles[ family ];
            if (dict.contains(name)) {
                kDebug(30003) << "Style: '" << name << "' already exists";
                delete dict.take(name);
            }
            dict.insert(name, new KoXmlElement(e));
            //kDebug(30003) <<"Style: '" << name <<"' loaded";
        }
    } else if (ns == KoXmlNS::style && (
            localName == "page-layout"
            || localName == "font-face")) {
        if (d->styles.contains(name)) {
            kDebug(30003) << "Style: '" << name << "' already exists";
            delete d->styles.take(name);
        }
        d->styles.insert(name, new KoXmlElement(e));
    } else if (localName == "presentation-page-layout" && ns == KoXmlNS::style) {
        if (d->presentationPageLayouts.contains(name)) {
            kDebug(30003) << "Presentation page layout: '" << name << "' already exists";
            delete d->presentationPageLayouts.take(name);
        }
        d->presentationPageLayouts.insert(name, new KoXmlElement(e));
    } else if (localName == "default-style" && ns == KoXmlNS::style) {
        const QString family = e.attributeNS(KoXmlNS::style, "family", QString());
        if (!family.isEmpty())
            d->defaultStyles.insert(family, new KoXmlElement(e));
    } else if (ns == KoXmlNS::number && (
                   localName == "number-style"
                   || localName == "currency-style"
                   || localName == "percentage-style"
                   || localName == "boolean-style"
                   || localName == "text-style"
                   || localName == "date-style"
                   || localName == "time-style")) {
        QPair<QString, KoOdfNumberStyles::NumericStyleFormat> numberStyle = KoOdfNumberStyles::loadOdfNumberStyle(e);
        d->dataFormats.insert(numberStyle.first, qMakePair(numberStyle.second, new KoXmlElement(e)));
    } else if (ns == KoXmlNS::text && localName == "notes-configuration") {
        KoOdfNotesConfiguration notesConfiguration;
        notesConfiguration.loadOdf(e);
        if (notesConfiguration.noteClass() == KoOdfNotesConfiguration::Footnote) {
            d->globalFootnoteConfiguration = notesConfiguration;
        }
        else if (notesConfiguration.noteClass() == KoOdfNotesConfiguration::Endnote) {
            d->globalEndnoteConfiguration = notesConfiguration;
        }
    } else if (ns == KoXmlNS::text && localName == "linenumbering-configuration") {
        d->lineNumberingConfiguration.loadOdf(e);
    }

}

KoXmlElement *KoOdfStylesReader::defaultStyle(const QString &family) const
{
    return d->defaultStyles[family];
}

KoXmlElement KoOdfStylesReader::officeStyle() const
{
    return d->officeStyle;
}

KoXmlElement KoOdfStylesReader::layerSet() const
{
    return d->layerSet;
}

QHash<QString, KoXmlElement*> KoOdfStylesReader::masterPages() const
{
    return d->masterPages;
}

QHash<QString, KoXmlElement*> KoOdfStylesReader::presentationPageLayouts() const
{
    return d->presentationPageLayouts;
}

QHash<QString, KoXmlElement*> KoOdfStylesReader::drawStyles() const
{
    return d->drawStyles;
}

const KoXmlElement* KoOdfStylesReader::findStyle(const QString& name) const
{
    return d->styles[ name ];
}

const KoXmlElement* KoOdfStylesReader::findStyle(const QString& styleName, const QString& family) const
{
    const KoXmlElement* style = findStyleCustomStyle(styleName, family);
    if (!style)
        style = findAutoStyleStyle(styleName, family);
    if (!style)
        style = findContentAutoStyle(styleName, family);
    return style;
}

const KoXmlElement* KoOdfStylesReader::findStyle(const QString& styleName, const QString& family, bool stylesDotXml) const
{
    const KoXmlElement* style = findStyleCustomStyle(styleName, family);
    if (!style && !stylesDotXml) {
        style = findContentAutoStyle(styleName, family);
    }
    if (!style && stylesDotXml) {
        style = findAutoStyleStyle(styleName, family);
    }
    return style;

}

const KoXmlElement* KoOdfStylesReader::findStyleCustomStyle(const QString& styleName, const QString& family) const
{
    const KoXmlElement* style = d->customStyles.value(family).value(styleName);
    if (style && !family.isEmpty()) {
        const QString styleFamily = style->attributeNS(KoXmlNS::style, "family", QString());
        if (styleFamily != family) {
            kWarning() << "KoOdfStylesReader: was looking for style " << styleName
                    << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KoXmlElement* KoOdfStylesReader::findAutoStyleStyle(const QString& styleName, const QString& family) const
{
    const KoXmlElement* style = d->stylesAutoStyles.value(family).value(styleName);
    if (style) {
        const QString styleFamily = style->attributeNS(KoXmlNS::style, "family", QString());
        if (styleFamily != family) {
            kWarning() << "KoOdfStylesReader: was looking for style " << styleName
                    << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KoXmlElement* KoOdfStylesReader::findContentAutoStyle(const QString& styleName, const QString& family) const
{
    const KoXmlElement* style = d->contentAutoStyles.value(family).value(styleName);
    if (style) {
        const QString styleFamily = style->attributeNS(KoXmlNS::style, "family", QString());
        if (styleFamily != family) {
            kWarning() << "KoOdfStylesReader: was looking for style " << styleName
                    << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}
