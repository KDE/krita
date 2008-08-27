/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextSharedLoadingData.h"

#include <QString>
#include <QHash>

#include <kdebug.h>

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoOdfStylesReader.h>
#include <KoOdfLoadingContext.h>

#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"

class KoTextSharedLoadingData::Private
{
public:
    ~Private() {
        qDeleteAll(paragraphStylesToDelete);
        qDeleteAll(characterStylesToDelete);
    }

    // It is possible that automatic-styles in content.xml and styles.xml have the same name
    // within the same family. Therefore we have to keep them separate. The office:styles are
    // added to the autostyles so that only one lookup is needed to get the style. This is
    // about 30% faster than having a special data structure for office:styles.
    QHash<QString, KoParagraphStyle *> paragraphContentDotXmlStyles;
    QHash<QString, KoCharacterStyle *> characterContentDotXmlStyles;
    QHash<QString, KoListStyle *>      listContentDotXmlStyles;
    QHash<QString, KoParagraphStyle *> paragraphStylesDotXmlStyles;
    QHash<QString, KoCharacterStyle *> characterStylesDotXmlStyles;
    QHash<QString, KoListStyle *>      listStylesDotXmlStyles;

    QList<KoParagraphStyle *> paragraphStylesToDelete;
    QList<KoCharacterStyle *> characterStylesToDelete;
};

KoTextSharedLoadingData::KoTextSharedLoadingData()
        : d(new Private())
{
}

KoTextSharedLoadingData::~KoTextSharedLoadingData()
{
    delete d;
}

static void addDefaultParagraphStyle(KoOdfLoadingContext & context, const KoXmlElement* styleElem, KoStyleManager* styleManager)
{
    if (styleManager && styleElem) {
        styleManager->defaultParagraphStyle()->loadOdf(styleElem, context);
    }
}

void KoTextSharedLoadingData::loadOdfStyles(KoOdfLoadingContext & context, KoStyleManager * styleManager)
{
    addCharacterStyles(context, context.stylesReader().autoStyles("text").values(), ContentDotXml);
    addCharacterStyles(context, context.stylesReader().autoStyles("text", true).values(), StylesDotXml);
    // only add styles of office:styles to the style manager
    addCharacterStyles(context, context.stylesReader().customStyles("text").values(), ContentDotXml | StylesDotXml, styleManager);

    addListStyles(context, context.stylesReader().autoStyles("list").values(), ContentDotXml, styleManager, true);
    addListStyles(context, context.stylesReader().autoStyles("list", true).values(), StylesDotXml, styleManager, true);
    addListStyles(context, context.stylesReader().customStyles("list").values(), ContentDotXml | StylesDotXml, styleManager, false);

    // add office:automatic-styles in content.xml to paragraphContentDotXmlStyles
    addParagraphStyles(context, context.stylesReader().autoStyles("paragraph").values(), ContentDotXml);
    // add office:automatic-styles in styles.xml to paragraphStylesDotXmlStyles
    addParagraphStyles(context, context.stylesReader().autoStyles("paragraph", true).values(), StylesDotXml);
    // add office:styles from styles.xml to paragraphContentDotXmlStyles, paragraphStylesDotXmlStyles and styleManager
    // now all styles referencable from the body in content.xml is in paragraphContentDotXmlStyles
    addParagraphStyles(context, context.stylesReader().customStyles("paragraph").values(), ContentDotXml | StylesDotXml, styleManager);
    addDefaultParagraphStyle(context, context.stylesReader().defaultStyle("paragraph"), styleManager);

    addOutlineStyle(context, styleManager);

    kDebug(32500) << "content.xml: paragraph styles" << d->paragraphContentDotXmlStyles.count() << "character styles" << d->characterContentDotXmlStyles.count();
    kDebug(32500) << "styles.xml:  paragraph styles" << d->paragraphStylesDotXmlStyles.count() << "character styles" << d->characterStylesDotXmlStyles.count();
}

void KoTextSharedLoadingData::addParagraphStyles(KoOdfLoadingContext & context, QList<KoXmlElement*> styleElements,
        int styleTypes, KoStyleManager * styleManager)
{
    QList<QPair<QString, KoParagraphStyle *> > paragraphStyles(loadParagraphStyles(context, styleElements, styleTypes, styleManager));

    QList<QPair<QString, KoParagraphStyle *> >::iterator it(paragraphStyles.begin());
    for (; it != paragraphStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->paragraphContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->paragraphStylesDotXmlStyles.insert(it->first, it->second);
        }

        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->paragraphStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KoParagraphStyle *> > KoTextSharedLoadingData::loadParagraphStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
        int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoParagraphStyle *> > paragraphStyles;

    foreach(KoXmlElement* styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        KoParagraphStyle *parastyle = new KoParagraphStyle();
        parastyle->loadOdf(styleElem, context);
        QString listStyleName = styleElem->attributeNS(KoXmlNS::style, "list-style-name", QString());
        KoListStyle *list = listStyle(listStyleName, styleTypes & StylesDotXml);
        if (!list && styleManager)
            list = styleManager->defaultListStyle();
        if (list)
            parastyle->setListStyle(*list);
        paragraphStyles.append(QPair<QString, KoParagraphStyle *>(name, parastyle));
    }
    return paragraphStyles;
}

void KoTextSharedLoadingData::addCharacterStyles(KoOdfLoadingContext & context, QList<KoXmlElement*> styleElements,
        int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoCharacterStyle *> > characterStyles(loadCharacterStyles(context, styleElements));

    QList<QPair<QString, KoCharacterStyle *> >::iterator it(characterStyles.begin());
    for (; it != characterStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->characterContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->characterStylesDotXmlStyles.insert(it->first, it->second);
        }

        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->characterStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KoCharacterStyle *> > KoTextSharedLoadingData::loadCharacterStyles(KoOdfLoadingContext & context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoCharacterStyle *> > characterStyles;

    foreach(KoXmlElement* styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        QString displayName = styleElem->attributeNS(KoXmlNS::style, "display-name", QString());
        if (displayName.isEmpty()) {
            displayName = name;
        }

        kDebug(32500) << "styleName =" << name << "styleDisplayName =" << displayName;

        context.styleStack().save();
        context.addStyles(styleElem, "text");   // Load all parents - only because we don't support inheritance.

        context.styleStack().setTypeProperties("text");

        KoCharacterStyle *characterStyle = new KoCharacterStyle();
        characterStyle->setName(displayName);
        characterStyle->loadOdf(context);

        context.styleStack().restore();

        characterStyles.append(QPair<QString, KoCharacterStyle *>(name, characterStyle));
    }
    return characterStyles;
}

void KoTextSharedLoadingData::addListStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
        int styleTypes, KoStyleManager *styleManager, bool automaticStyle)
{
    QList<QPair<QString, KoListStyle *> > listStyles(loadListStyles(context, styleElements));

    QList<QPair<QString, KoListStyle *> >::iterator it(listStyles.begin());
    for (; it != listStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->listContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->listStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            if (automaticStyle) {
                styleManager->addAutomaticListStyle(it->second);
            } else {
                styleManager->add(it->second);
            }
        }
    }
}

QList<QPair<QString, KoListStyle *> > KoTextSharedLoadingData::loadListStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoListStyle *> > listStyles;

    foreach(KoXmlElement* styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        KoListStyle *liststyle = new KoListStyle();
        liststyle->loadOdf(context, *styleElem);
        listStyles.append(QPair<QString, KoListStyle *>(name, liststyle));
    }
    return listStyles;
}

void KoTextSharedLoadingData::addOutlineStyle(KoOdfLoadingContext & context, KoStyleManager *styleManager)
{
    // outline-styles used e.g. for headers
    KoXmlElement outlineStyleElem = KoXml::namedItemNS(context.stylesReader().officeStyle(), KoXmlNS::text, "outline-style");
    if (styleManager && outlineStyleElem.isElement()) {
        KoListStyle *outlineStyle = new KoListStyle();
        outlineStyle->loadOdf(context, outlineStyleElem);
        styleManager->setOutlineStyle(outlineStyle); // style manager owns it now. he will take care of deleting it.
    }
}

KoParagraphStyle * KoTextSharedLoadingData::paragraphStyle(const QString &name, bool stylesDotXml)
{
    return stylesDotXml ? d->paragraphStylesDotXmlStyles.value(name) : d->paragraphContentDotXmlStyles.value(name);
}

KoCharacterStyle * KoTextSharedLoadingData::characterStyle(const QString &name, bool stylesDotXml)
{
    return stylesDotXml ? d->characterStylesDotXmlStyles.value(name) : d->characterContentDotXmlStyles.value(name);
}

KoListStyle * KoTextSharedLoadingData::listStyle(const QString &name, bool stylesDotXml)
{
    return stylesDotXml ? d->listStylesDotXmlStyles.value(name) : d->listContentDotXmlStyles.value(name);
}

