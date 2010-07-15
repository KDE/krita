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
#include <KoShapeLoadingContext.h>

#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "styles/KoTableStyle.h"
#include "styles/KoTableColumnStyle.h"
#include "styles/KoTableRowStyle.h"
#include "styles/KoTableCellStyle.h"
#include "styles/KoSectionStyle.h"

class KoTextSharedLoadingData::Private
{
public:
    ~Private() {
        qDeleteAll(paragraphStylesToDelete);
        qDeleteAll(characterStylesToDelete);
        qDeleteAll(listStylesToDelete);
        qDeleteAll(tableStylesToDelete);
        qDeleteAll(tableCellStylesToDelete);
        qDeleteAll(sectionStylesToDelete);
        // NOTE: Table column/row styles are managed by KoTableColumnAndRowStyleManager
        //       and are not deleted here.
    }

    // It is possible that automatic-styles in content.xml and styles.xml have the same name
    // within the same family. Therefore we have to keep them separate. The office:styles are
    // added to the autostyles so that only one lookup is needed to get the style. This is
    // about 30% faster than having a special data structure for office:styles.
    QHash<QString, KoParagraphStyle *> paragraphContentDotXmlStyles;
    QHash<QString, KoCharacterStyle *> characterContentDotXmlStyles;
    QHash<QString, KoListStyle *>      listContentDotXmlStyles;
    QHash<QString, KoTableStyle *>      tableContentDotXmlStyles;
    QHash<QString, KoTableColumnStyle *>      tableColumnContentDotXmlStyles;
    QHash<QString, KoTableRowStyle *>      tableRowContentDotXmlStyles;
    QHash<QString, KoTableCellStyle *>      tableCellContentDotXmlStyles;
    QHash<QString, KoSectionStyle *>      sectionContentDotXmlStyles;
    QHash<QString, KoParagraphStyle *> paragraphStylesDotXmlStyles;
    QHash<QString, KoCharacterStyle *> characterStylesDotXmlStyles;
    QHash<QString, KoListStyle *>      listStylesDotXmlStyles;
    QHash<QString, KoTableStyle *>      tableStylesDotXmlStyles;
    QHash<QString, KoTableColumnStyle *>      tableColumnStylesDotXmlStyles;
    QHash<QString, KoTableRowStyle *>      tableRowStylesDotXmlStyles;
    QHash<QString, KoTableCellStyle *>      tableCellStylesDotXmlStyles;
    QHash<QString, KoSectionStyle *>      sectionStylesDotXmlStyles;

    QList<KoParagraphStyle *> paragraphStylesToDelete;
    QList<KoCharacterStyle *> characterStylesToDelete;
    QList<KoListStyle *> listStylesToDelete;
    QList<KoTableStyle *> tableStylesToDelete;
    QList<KoTableCellStyle *> tableCellStylesToDelete;
    QList<KoSectionStyle *> sectionStylesToDelete;
    QHash<QString, KoParagraphStyle*> namedParagraphStyles;
};

KoTextSharedLoadingData::KoTextSharedLoadingData()
        : d(new Private())
{
}

KoTextSharedLoadingData::~KoTextSharedLoadingData()
{
    delete d;
}

void KoTextSharedLoadingData::addDefaultParagraphStyle(KoShapeLoadingContext &context, const KoXmlElement *styleElem, const KoXmlElement *appDefault, KoStyleManager *styleManager)
{
    if (styleManager && styleElem) {
        styleManager->defaultParagraphStyle()->loadOdf(styleElem, context);
    }
    else if (styleManager && appDefault) {
        styleManager->defaultParagraphStyle()->loadOdf(appDefault, context);
    }
}

void KoTextSharedLoadingData::loadOdfStyles(KoShapeLoadingContext &scontext, KoStyleManager *styleManager)
{
    KoOdfLoadingContext &context = scontext.odfLoadingContext();

    addCharacterStyles(context, context.stylesReader().autoStyles("text").values(), ContentDotXml);
    addCharacterStyles(context, context.stylesReader().autoStyles("text", true).values(), StylesDotXml);
    // only add styles of office:styles to the style manager
    addCharacterStyles(context, context.stylesReader().customStyles("text").values(), ContentDotXml | StylesDotXml, styleManager);

    addListStyles(scontext, context.stylesReader().autoStyles("list").values(), ContentDotXml);
    addListStyles(scontext, context.stylesReader().autoStyles("list", true).values(), StylesDotXml);
    addListStyles(scontext, context.stylesReader().customStyles("list").values(), ContentDotXml | StylesDotXml, styleManager);

    addDefaultParagraphStyle(scontext, context.stylesReader().defaultStyle("paragraph"), context.defaultStylesReader().defaultStyle("paragraph"), styleManager);
    // adding all the styles in order of dependency; automatic styles can have a parent in the named styles, so load the named styles first.

    // add office:styles from styles.xml to paragraphContentDotXmlStyles, paragraphStylesDotXmlStyles and styleManager
    // now all styles referencable from the body in content.xml is in paragraphContentDotXmlStyles
    addParagraphStyles(scontext, context.stylesReader().customStyles("paragraph").values(), ContentDotXml | StylesDotXml, styleManager);
    // add office:automatic-styles in styles.xml to paragraphStylesDotXmlStyles
    addParagraphStyles(scontext, context.stylesReader().autoStyles("paragraph", true).values(), StylesDotXml);
    // add office:automatic-styles in content.xml to paragraphContentDotXmlStyles
    addParagraphStyles(scontext, context.stylesReader().autoStyles("paragraph").values(), ContentDotXml);

    addTableStyles(context, context.stylesReader().autoStyles("table").values(), ContentDotXml);
    addTableStyles(context, context.stylesReader().autoStyles("table", true).values(), StylesDotXml);
    addTableStyles(context, context.stylesReader().customStyles("table").values(), ContentDotXml | StylesDotXml, styleManager);

    addTableColumnStyles(context, context.stylesReader().autoStyles("table-column").values(), ContentDotXml);
    addTableColumnStyles(context, context.stylesReader().autoStyles("table-column", true).values(), StylesDotXml);
    addTableColumnStyles(context, context.stylesReader().customStyles("table-column").values(), ContentDotXml | StylesDotXml, styleManager);

    addTableRowStyles(context, context.stylesReader().autoStyles("table-row").values(), ContentDotXml);
    addTableRowStyles(context, context.stylesReader().autoStyles("table-row", true).values(), StylesDotXml);
    addTableRowStyles(context, context.stylesReader().customStyles("table-row").values(), ContentDotXml | StylesDotXml, styleManager);

    addTableCellStyles(context, context.stylesReader().autoStyles("table-cell").values(), ContentDotXml);
    addTableCellStyles(context, context.stylesReader().autoStyles("table-cell", true).values(), StylesDotXml);
    addTableCellStyles(context, context.stylesReader().customStyles("table-cell").values(), ContentDotXml | StylesDotXml, styleManager);

    addSectionStyles(context, context.stylesReader().autoStyles("section").values(), ContentDotXml);
    addSectionStyles(context, context.stylesReader().autoStyles("section", true).values(), StylesDotXml);
    addSectionStyles(context, context.stylesReader().customStyles("section").values(), ContentDotXml | StylesDotXml, styleManager);

    addOutlineStyle(scontext, styleManager);

    kDebug(32500) << "content.xml: paragraph styles" << d->paragraphContentDotXmlStyles.count() << "character styles" << d->characterContentDotXmlStyles.count();
    kDebug(32500) << "styles.xml:  paragraph styles" << d->paragraphStylesDotXmlStyles.count() << "character styles" << d->characterStylesDotXmlStyles.count();
}

void KoTextSharedLoadingData::addParagraphStyles(KoShapeLoadingContext &context, QList<KoXmlElement*> styleElements,
        int styleTypes, KoStyleManager *styleManager)
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
    }
}

QList<QPair<QString, KoParagraphStyle *> > KoTextSharedLoadingData::loadParagraphStyles(KoShapeLoadingContext &context, QList<KoXmlElement*> styleElements,
        int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoParagraphStyle *> > paragraphStyles;
    QHash<KoParagraphStyle*,QString> nextStyles;
    QHash<KoParagraphStyle*,QString> parentStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        KoParagraphStyle *parastyle = new KoParagraphStyle();
        parastyle->loadOdf(styleElem, context);
        QString listStyleName = styleElem->attributeNS(KoXmlNS::style, "list-style-name", QString());
        KoListStyle *list = listStyle(listStyleName, styleTypes & StylesDotXml);
        if (list) {
            KoListStyle *newListStyle = new KoListStyle(parastyle);
            newListStyle->copyProperties(list);
            parastyle->setListStyle(newListStyle);
        }
        paragraphStyles.append(QPair<QString, KoParagraphStyle *>(name, parastyle));
        d->namedParagraphStyles.insert(name, parastyle);

        if (styleElem->hasAttributeNS(KoXmlNS::style, "next-style-name"))
            nextStyles.insert(parastyle, styleElem->attributeNS(KoXmlNS::style, "next-style-name"));
        if (styleElem->hasAttributeNS(KoXmlNS::style, "parent-style-name"))
            parentStyles.insert(parastyle, styleElem->attributeNS(KoXmlNS::style, "parent-style-name"));

        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager)
            styleManager->add(parastyle);
        else
            d->paragraphStylesToDelete.append(parastyle);
    }

    // second pass; resolve all the 'next-style's and parent-style's.
    foreach (KoParagraphStyle *style, nextStyles.keys()) {
        KoParagraphStyle *next = d->namedParagraphStyles.value(nextStyles.value(style));
        if (next && next->styleId() >= 0)
            style->setNextStyle(next->styleId());
    }
    foreach (KoParagraphStyle *style, parentStyles.keys()) {
        KoParagraphStyle *parent = d->namedParagraphStyles.value(parentStyles.value(style));
        if (parent)
            style->setParentStyle(parent);
    }

    return paragraphStyles;
}

void KoTextSharedLoadingData::addCharacterStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
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

QList<QPair<QString, KoCharacterStyle *> > KoTextSharedLoadingData::loadCharacterStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoCharacterStyle *> > characterStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
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

void KoTextSharedLoadingData::addListStyles(KoShapeLoadingContext &context, QList<KoXmlElement*> styleElements,
                                            int styleTypes, KoStyleManager *styleManager)
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
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->listStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KoListStyle *> > KoTextSharedLoadingData::loadListStyles(KoShapeLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoListStyle *> > listStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        KoListStyle *liststyle = new KoListStyle();
        liststyle->loadOdf(context, *styleElem);
        listStyles.append(QPair<QString, KoListStyle *>(name, liststyle));
    }
    return listStyles;
}

void KoTextSharedLoadingData::addTableStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
                                            int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoTableStyle *> > tableStyles(loadTableStyles(context, styleElements));

    QList<QPair<QString, KoTableStyle *> >::iterator it(tableStyles.begin());
    for (; it != tableStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->tableStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KoTableStyle *> > KoTextSharedLoadingData::loadTableStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoTableStyle *> > tableStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KoXmlNS::style, "style-name", "table");
        KoTableStyle *tablestyle = new KoTableStyle();
        tablestyle->loadOdf(styleElem, context);
        tableStyles.append(QPair<QString, KoTableStyle *>(name, tablestyle));
    }
    return tableStyles;
}

void KoTextSharedLoadingData::addTableColumnStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
                                            int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoTableColumnStyle *> > tableColumnStyles(loadTableColumnStyles(context, styleElements));

    QList<QPair<QString, KoTableColumnStyle *> >::iterator it(tableColumnStyles.begin());
    for (; it != tableColumnStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableColumnContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableColumnStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they will be managed by the
        // tableColumnAndRowStyleManager instead
        if (styleManager) {
            styleManager->add(it->second);
        }
    }
}

QList<QPair<QString, KoTableColumnStyle *> > KoTextSharedLoadingData::loadTableColumnStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoTableColumnStyle *> > tableColumnStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KoXmlNS::style, "style-name", "table");
        KoTableColumnStyle *tablecolumnstyle = new KoTableColumnStyle();
        tablecolumnstyle->loadOdf(styleElem, context);
        tableColumnStyles.append(QPair<QString, KoTableColumnStyle *>(name, tablecolumnstyle));
    }
    return tableColumnStyles;
}

void KoTextSharedLoadingData::addTableRowStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
                                            int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoTableRowStyle *> > tableRowStyles(loadTableRowStyles(context, styleElements));

    QList<QPair<QString, KoTableRowStyle *> >::iterator it(tableRowStyles.begin());
    for (; it != tableRowStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableRowContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableRowStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they will be managed by the
        // tableColumnAndRowStyleManager instead
        if (styleManager) {
            styleManager->add(it->second);
        }
    }
}

QList<QPair<QString, KoTableRowStyle *> > KoTextSharedLoadingData::loadTableRowStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoTableRowStyle *> > tableRowStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KoXmlNS::style, "style-name", "table");
        KoTableRowStyle *tablerowstyle = new KoTableRowStyle();
        tablerowstyle->loadOdf(styleElem, context);
        tableRowStyles.append(QPair<QString, KoTableRowStyle *>(name, tablerowstyle));
    }
    return tableRowStyles;
}

void KoTextSharedLoadingData::addTableCellStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
                                            int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoTableCellStyle *> > tableCellStyles(loadTableCellStyles(context, styleElements));

    QList<QPair<QString, KoTableCellStyle *> >::iterator it(tableCellStyles.begin());
    for (; it != tableCellStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->tableCellContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->tableCellStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->tableCellStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KoTableCellStyle *> > KoTextSharedLoadingData::loadTableCellStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoTableCellStyle *> > tableCellStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KoXmlNS::style, "style-name", "table");
        KoTableCellStyle *tablecellstyle = new KoTableCellStyle();
        tablecellstyle->loadOdf(styleElem, context);
        tableCellStyles.append(QPair<QString, KoTableCellStyle *>(name, tablecellstyle));
    }
    return tableCellStyles;
}

void KoTextSharedLoadingData::addSectionStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
                                            int styleTypes, KoStyleManager *styleManager)
{
    QList<QPair<QString, KoSectionStyle *> > sectionStyles(loadSectionStyles(context, styleElements));

    QList<QPair<QString, KoSectionStyle *> >::iterator it(sectionStyles.begin());
    for (; it != sectionStyles.end(); ++it) {
        if (styleTypes & ContentDotXml) {
            d->sectionContentDotXmlStyles.insert(it->first, it->second);
        }
        if (styleTypes & StylesDotXml) {
            d->sectionStylesDotXmlStyles.insert(it->first, it->second);
        }
        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if (styleManager) {
            styleManager->add(it->second);
        } else {
            d->sectionStylesToDelete.append(it->second);
        }
    }
}

QList<QPair<QString, KoSectionStyle *> > KoTextSharedLoadingData::loadSectionStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements)
{
    QList<QPair<QString, KoSectionStyle *> > sectionStyles;

    foreach(KoXmlElement *styleElem, styleElements) {
        Q_ASSERT(styleElem);
        Q_ASSERT(!styleElem->isNull());

        QString name = styleElem->attributeNS(KoXmlNS::style, "name", QString());
        // nah don't think this is it: context.fillStyleStack(*styleElem, KoXmlNS::style, "style-name", "table");
        KoSectionStyle *sectionstyle = new KoSectionStyle();
        sectionstyle->loadOdf(styleElem, context);
        sectionStyles.append(QPair<QString, KoSectionStyle *>(name, sectionstyle));
    }
    return sectionStyles;
}

void KoTextSharedLoadingData::addOutlineStyle(KoShapeLoadingContext &context, KoStyleManager *styleManager)
{
    // outline-styles used e.g. for headers
    KoXmlElement outlineStyleElem = KoXml::namedItemNS(context.odfLoadingContext().stylesReader().officeStyle(), KoXmlNS::text, "outline-style");
    if (styleManager && outlineStyleElem.isElement()) {
        KoListStyle *outlineStyle = new KoListStyle();
        outlineStyle->loadOdf(context, outlineStyleElem);
        styleManager->setOutlineStyle(outlineStyle); // style manager owns it now. he will take care of deleting it.
    }
}

KoParagraphStyle *KoTextSharedLoadingData::paragraphStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->paragraphStylesDotXmlStyles.value(name) : d->paragraphContentDotXmlStyles.value(name);
}

KoCharacterStyle *KoTextSharedLoadingData::characterStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->characterStylesDotXmlStyles.value(name) : d->characterContentDotXmlStyles.value(name);
}

QList<KoCharacterStyle*> KoTextSharedLoadingData::characterStyles(bool stylesDotXml) const
{
    return stylesDotXml ? d->characterStylesDotXmlStyles.values() : d->characterContentDotXmlStyles.values();
}

KoListStyle *KoTextSharedLoadingData::listStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->listStylesDotXmlStyles.value(name) : d->listContentDotXmlStyles.value(name);
}

KoTableStyle *KoTextSharedLoadingData::tableStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableStylesDotXmlStyles.value(name) : d->tableContentDotXmlStyles.value(name);
}

KoTableColumnStyle *KoTextSharedLoadingData::tableColumnStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableColumnStylesDotXmlStyles.value(name) : d->tableColumnContentDotXmlStyles.value(name);
}

KoTableRowStyle *KoTextSharedLoadingData::tableRowStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableRowStylesDotXmlStyles.value(name) : d->tableRowContentDotXmlStyles.value(name);
}

KoTableCellStyle *KoTextSharedLoadingData::tableCellStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->tableCellStylesDotXmlStyles.value(name) : d->tableCellContentDotXmlStyles.value(name);
}

KoSectionStyle *KoTextSharedLoadingData::sectionStyle(const QString &name, bool stylesDotXml) const
{
    return stylesDotXml ? d->sectionStylesDotXmlStyles.value(name) : d->sectionContentDotXmlStyles.value(name);
}

void KoTextSharedLoadingData::shapeInserted(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &/*context*/)
{
    Q_UNUSED(shape);
    Q_UNUSED(element);
}
