/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "KoOpenDocumentLoader.h"
//#include "KWDocument.h"
//#include "frames/KWTextFrameSet.h"
//#include "frames/KWTextFrame.h"

// koffice
#include <KoOasisLoadingContext.h>
#include <KoOasisStyles.h>
#include <KoOasisSettings.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>

#include "../styles/KoStyleManager.h"
#include "../styles/KoParagraphStyle.h"
#include "../styles/KoCharacterStyle.h"
#include "../styles/KoListStyle.h"
#include "../styles/KoListLevelProperties.h"
#include <KoPageLayout.h>

// KDE + Qt includes
#include <QDomDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <klocale.h>

// if defined then debugging is enabled
#define KOOPENDOCUMENTLOADER_DEBUG

/// \internal d-pointer class.
class KoOpenDocumentLoader::Private
{
    public:
        explicit Private() {}
        ~Private() {
            qDeleteAll(listStyles);
        }

        KoStyleManager* stylemanager;

        KoParagraphStyle *paragraphStyle(const QString &name) {
            if (paragraphStyles.contains(name))
                return paragraphStyles[name];
            KoParagraphStyle *style = stylemanager->paragraphStyle(name);
            if (style)
                paragraphStyles[name] = style;
            return style;
        }
        KoCharacterStyle *characterStyle(const QString &name) {
            if (characterStyles.contains(name))
                return characterStyles[name];
            KoCharacterStyle *style = stylemanager->characterStyle(name);
            if (style)
                characterStyles[name] = style;
            return style;
        }
        KoListStyle *listStyle(const QString &name) {
            return listStyles.contains(name) ? listStyles[name] : 0;
        }

        void addStyle (KoParagraphStyle *style) {
            stylemanager->add(style);
            paragraphStyles[style->name()] = style;
        }
        void addStyle (KoCharacterStyle *style) {
            stylemanager->add(style);
            characterStyles[style->name()] = style;
        }
        void addStyle (KoListStyle *style) {
            listStyles[style->name()] = style;
        }

    private:
        QHash<QString, KoParagraphStyle *>paragraphStyles;
        QHash<QString, KoCharacterStyle *>characterStyles;
        QHash<QString, KoListStyle *>listStyles;
};

KoOpenDocumentLoader::KoOpenDocumentLoader(KoStyleManager* stylemanager)
    : QObject(), d(new Private())
{
    d->stylemanager = stylemanager;
}

KoOpenDocumentLoader::~KoOpenDocumentLoader()
{
    delete d;
}

KoStyleManager* KoOpenDocumentLoader::styleManager() const
{
    return d->stylemanager;
}

//1.6: KoStyleCollection::loadOasisStyles
void KoOpenDocumentLoader::loadStyles(KoOasisLoadingContext& context, QList<KoXmlElement*> styleElements)
{
    foreach(KoXmlElement* styleElem, styleElements) {
        Q_ASSERT( styleElem );
        Q_ASSERT( !styleElem->isNull() );

        //1.6: KoParagStyle::loadStyle
        QString name = styleElem->attributeNS( KoXmlNS::style, "name", QString() );
        QString displayName = styleElem->attributeNS( KoXmlNS::style, "display-name", QString() );
        if ( displayName.isEmpty() )
            displayName = name;

        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug()<<"KoOpenDocumentLoader::loadStyles styleName="<<name<<" styleDisplayName="<<displayName<<endl;
        #endif
#if 0 //1.6:
        // OOo hack:
        //m_bOutline = name.startsWith( "Heading" );
        // real OASIS solution:
        bool m_bOutline = styleElem->hasAttributeNS( KoXmlNS::style, "default-outline-level" );
#endif
        context.styleStack().save();
        context.addStyles( styleElem, "paragraph" ); // Load all parents - only because we don't support inheritance.

        KoParagraphStyle *parastyle = new KoParagraphStyle();
        parastyle->setName(name);
        //parastyle->setParent( d->stylemanager->defaultParagraphStyle() );
        d->addStyle(parastyle);

        //1.6: KoTextParag::loadOasis => KoParagLayout::loadOasisParagLayout
        context.styleStack().setTypeProperties( "paragraph" ); // load all style attributes from "style:paragraph-properties"
        parastyle->loadOasis(context.styleStack()); // load the KoParagraphStyle from the stylestack

        //1.6: KoTextFormat::load
        KoCharacterStyle *charstyle = parastyle->characterStyle();
        context.styleStack().setTypeProperties( "text" ); // load all style attributes from "style:text-properties"
        charstyle->loadOasis(context); // load the KoCharacterStyle from the stylestack

        context.styleStack().restore();
    }
}

//1.6: KoStyleCollection::loadOasisStyles
void KoOpenDocumentLoader::loadAllStyles(KoOasisLoadingContext& context)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadAllStyles"<<endl;
    #endif
    // User styles are named and appear in the gui while automatic styles are just a way to
    // save formatting changes done by the user. There is no real tech diff between them
    // except how we present them to the user.
    loadStyles(context, context.oasisStyles().autoStyles("paragraph").values());
    loadStyles(context, context.oasisStyles().customStyles("paragraph").values());

    // handle the list styles
    QHash<QString, KoXmlElement*> listStyles = context.oasisStyles().listStyles();
    for(QHash<QString, KoXmlElement*>::Iterator it = listStyles.begin(); it != listStyles.end(); ++it) {
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug()<<"KoOpenDocumentLoader::loadAllStyles listStyle="<<it.key()<<endl;
        #endif
        KoListStyle* style = new KoListStyle();
        style->setName(it.key());
        style->loadOasis(context, *it.value());
        d->addStyle(style);
    }
}

void KoOpenDocumentLoader::loadSettings(KoOasisLoadingContext& context, const QDomDocument& settings)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadSettings"<<endl;
    #endif
    Q_UNUSED(context);
    Q_UNUSED(settings);
}

bool KoOpenDocumentLoader::loadPageLayout(KoOasisLoadingContext& context, const QString& masterPageName)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadPageLayout"<<endl;
    #endif
    Q_UNUSED(context);
    Q_UNUSED(masterPageName);
    return true;
}

bool KoOpenDocumentLoader::loadMasterPageStyle(KoOasisLoadingContext& context, const QString& masterPageName)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadMasterPageStyle"<<endl;
    #endif
    Q_UNUSED(context);
    Q_UNUSED(masterPageName);
    return true;
}

//1.6: KoTextDocument::loadOasisText
void KoOpenDocumentLoader::loadBody(KoOasisLoadingContext& context, const KoXmlElement& bodyElem, QTextCursor& cursor)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadBody"<<endl;
    #endif

    startBody( bodyElem.childNodes().count() );
    for(KoXmlNode node = bodyElem.firstChild(); ! node.isNull(); node = node.nextSibling()) {
        KoXmlElement tag = node.toElement();
        if( ! tag.isNull() ) {
            context.styleStack().save();
            const QString localName = tag.localName();
            const bool isTextNS = ( tag.namespaceURI() == KoXmlNS::text );

            if ( isTextNS && localName == "p" ) {  // text paragraph
                loadParagraph(context, tag, cursor);
            }
            else if ( isTextNS && localName == "h" ) { // heading
                loadHeading(context, tag, cursor);
            }
            else if ( isTextNS &&
                    ( localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                        || localName == "list" || localName == "numbered-paragraph" ) ) { // OASIS
                loadList(context, tag, cursor);
            }
            else if ( isTextNS && localName == "section" ) { // Temporary support (###TODO)
                loadSection(context, tag, cursor);
            }
            else {
                #ifdef KOOPENDOCUMENTLOADER_DEBUG
                    kDebug()<<"Unhandled localName="<<localName<<endl;
                #endif
            }

            context.styleStack().restore(); // remove the styles added by the paragraph or list
        }
        processBody();
    }
    endBody();
}

//1.6: KoTextDocument::loadOasisText
void KoOpenDocumentLoader::loadParagraph(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );
    QString userStyleName = context.styleStack().userStyleName( "paragraph" );
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadParagraph userStyleName="<<userStyleName<<endl;
    #endif
    KoParagraphStyle *userStyle = d->paragraphStyle(userStyleName);
    //if( ! userStyle ) userStyle = d->stylemanager->defaultParagraphStyle();
    if( userStyle ) {
        context.styleStack().setTypeProperties( "paragraph" );
        QTextBlock block = cursor.block();
        userStyle->applyStyle(block);
    }

    //1.6: KWTextParag::loadOasis
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadParagraph styleName="<<styleName<<" userStyleName="<<userStyleName<<" userStyle="<<(userStyle?"YES":"NULL")<<endl;
    #endif
    if ( !styleName.isEmpty() ) {
        const QDomElement* paragraphStyle = context.oasisStyles().findStyle( styleName, "paragraph" );
        QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString() ) : QString();
        if ( masterPageName.isEmpty() )
            masterPageName = "Standard";
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug() << "KoOpenDocumentLoader::loadParagraph paragraphStyle.localName=" << (paragraphStyle ? paragraphStyle->localName() : "NULL") << " masterPageName=" << masterPageName << endl;
        #endif

        //QString styleName = context.styleStack().userStyleName( "paragraph" );
        //KoParagraphStyle *style = d->stylemanager->paragraphStyle(styleName);
        //if ( !style ) style = d->stylemanager->defaultParagraphStyle();

        //d->currentMasterPage = masterPageName; // do this first to avoid recursion
        context.styleStack().save();
        context.styleStack().setTypeProperties( "paragraph" );
        if( paragraphStyle )
            context.addStyles( paragraphStyle, "paragraph" );
        //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );

        KoParagraphStyle *style = d->paragraphStyle(styleName);
        if( ! style ) {
            style = d->stylemanager->defaultParagraphStyle();
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug() << "KoOpenDocumentLoader::loadParagraph using default style!" << endl;
            #endif
        }
        if ( style ) {
            QTextBlock block = cursor.block();
            style->applyStyle(block);
        }

#if 0 //1.6:
        // This is quite ugly... OOo stores the starting page-number in the first paragraph style...
        QString pageNumber = context.styleStack().attributeNS( KoXmlNS::style, "page-number" );
        if ( !pageNumber.isEmpty() ) doc->variableCollection()->variableSetting()->setStartingPageNumber( pageNumber.toInt() );
#endif
        context.styleStack().restore();

//is this needed here at all?
//loadPageLayout(context, masterPageName); // page layout
    }

    //KoTextParag::loadOasisSpan
    //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "text" ); //is this really needed?

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
    context.styleStack().setTypeProperties( "paragraph" );
    const QString textStyleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.oasisStyles().findStyle( textStyleName, "paragraph" );
    KoCharacterStyle *charstyle = 0;
    if( textStyleElem ) {
        context.addStyles( textStyleElem, "paragraph" );
        charstyle = d->characterStyle(textStyleName);
        if( ! charstyle ) {
            charstyle = new KoCharacterStyle();
            charstyle->setName(textStyleName);
            charstyle->loadOasis(context);
            d->addStyle(charstyle);
        }
        charstyle->applyStyle(&cursor);
    }
    bool stripLeadingSpace = true;
    loadSpan(context, parent, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf); // restore the cursor char format

    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);
}

//1.6: KoTextDocument::loadOasisText
void KoOpenDocumentLoader::loadHeading(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );
#if 0 //1.6:
    int level = tag.attributeNS( KoXmlNS::text, "outline-level", QString::null ).toInt();
    bool listOK = false;
    // When a heading is inside a list, it seems that the list prevails.
    // Example:
    //    <text:list text:style-name="Numbering 1">
    //      <text:list-item text:start-value="5">
    //        <text:h text:style-name="P2" text:level="4">The header</text:h>
    // where P2 has list-style-name="something else"
    // Result: the numbering of the header follows "Numbering 1".
    // So we use the style for the outline level only if we're not inside a list:
    //if ( !context.atStartOfListItem() )
    // === The new method for this is that we simply override it after loading.
    listOK = context.pushOutlineListLevelStyle( level );
    int restartNumbering = -1;
    if ( tag.hasAttributeNS( KoXmlNS::text, "start-value" ) ) // OASIS extension http://lists.oasis-open.org/archives/office/200310/msg00033.html
        restartNumbering = tag.attributeNS( KoXmlNS::text, "start-value", QString::null ).toInt();
    KoTextParag *parag = createParag( this, lastParagraph, nextParagraph );
    parag->loadOasis( tag, context, styleColl, pos );
    if ( !lastParagraph ) setFirstParag( parag ); // First parag
    lastParagraph = parag;
    if ( listOK ) {
        parag->applyListStyle( context, restartNumbering, true /*ordered*/, true /*heading*/, level );
        context.listStyleStack().pop();
    }
#else
    int level = parent.attributeNS( KoXmlNS::text, "outline-level", QString::null ).toInt();
    //1.6: KoOasisContext::pushOutlineListLevelStyle
    //KoXmlElement outlineStyle = KoDom::namedItemNS( oasisStyles().officeStyle(), KoXmlNS::text, "outline-style" );
    KoListStyle* listStyle = 0;
    if( level > 0 ) {
        listStyle = new KoListStyle();
        KoListLevelProperties props;
        props.setStyle( KoListStyle::DecimalItem );
        props.setDisplayLevel(level);
        listStyle->setLevel(props);
    }

    //1.6: KWTextParag::loadOasis
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadHeading style-name="<<styleName<<" outline-level="<<level<<endl;
    #endif
    if ( !styleName.isEmpty() ) {
        const QDomElement* paragraphStyle = context.oasisStyles().findStyle( styleName, "paragraph" );
        //QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString::null ) : QString::null;
        //if ( masterPageName.isEmpty() ) masterPageName = "Standard"; // Seems to be a builtin name for the default layout...
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug() << "KoOpenDocumentLoader::loadBody styleName=" << styleName << endl;
        #endif
        //context.styleStack().save();
        context.styleStack().setTypeProperties( "paragraph" );
        if( paragraphStyle )
            context.addStyles( paragraphStyle, "paragraph" );
        //context.styleStack().restore();
        //loadPageLayout( masterPageName, context ); // page layout
    }
    else if( level > 0 ) //FIXME: this should work (as in add a new style to the Paragraph Style list of KWord
        styleName = QString("Heading%1").arg(level);

    KoParagraphStyle *paragStyle = d->paragraphStyle(styleName);
    if( ! paragStyle ) {
        paragStyle = new KoParagraphStyle();
        paragStyle->setName(styleName);
        d->addStyle(paragStyle);
        //paragStyle->loadOasis(context.styleStack());
        //KoCharacterStyle *charstyle = paragStyle->characterStyle();
        //charstyle->loadOasis(context);
    }

    if( listStyle ) {
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug()<<"KoOpenDocumentLoader::loadHeading with listStyle !"<<endl;
        #endif
        paragStyle->setListStyle(*listStyle);
        delete listStyle;
    }

    QTextBlock block = cursor.block();
    paragStyle->applyStyle(block);
#endif

    //1.6: KoTextParag::loadOasisSpan
    bool stripLeadingSpace = true;
    loadSpan(context, parent, cursor, &stripLeadingSpace);

    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);
}

//1.6: KoTextDocument::loadList
void KoOpenDocumentLoader::loadList(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
#if 0 //1.6:
    const QString oldListStyleName = context.currentListStyleName();
    if ( list.hasAttributeNS( KoXmlNS::text, "style-name" ) ) context.setCurrentListStyleName( list.attributeNS( KoXmlNS::text, "style-name", QString::null ) );
    bool listOK = !context.currentListStyleName().isEmpty();
    int level = ( list.localName() == "numbered-paragraph" ) ? list.attributeNS( KoXmlNS::text, "level", "1" ).toInt() : context.listStyleStack().level() + 1;
    if ( listOK ) listOK = context.pushListLevelStyle( context.currentListStyleName(), level );
    const QDomElement listStyle = context.listStyleStack().currentListStyle();
    // The tag is either list-level-style-number or list-level-style-bullet
    const bool orderedList = listStyle.localName() == "list-level-style-number";
    if ( list.localName() == "numbered-paragraph" ) {
        // A numbered-paragraph contains paragraphs directly (it's both a list and a list-item)
        int restartNumbering = -1;
        if ( list.hasAttributeNS( KoXmlNS::text, "start-value" ) ) restartNumbering = list.attributeNS( KoXmlNS::text, "start-value", QString::null ).toInt();
        KoTextParag* oldLast = lastParagraph;
        lastParagraph = loadOasisText( list, context, lastParagraph, styleColl, nextParagraph );
        KoTextParag* firstListItem = oldLast ? oldLast->next() : firstParag();
        // Apply list style to first paragraph inside numbered-parag - there's only one anyway
        // Keep the "is outline" property though
        bool isOutline = firstListItem->counter() && firstListItem->counter()->numbering() == KoParagCounter::NUM_CHAPTER;
        firstListItem->applyListStyle( context, restartNumbering, orderedList, isOutline, level );
    } else {
        for ( QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling() ) {
            QDomElement listItem = n.toElement();
            int restartNumbering = -1;
            if ( listItem.hasAttributeNS( KoXmlNS::text, "start-value" ) ) restartNumbering = listItem.attributeNS( KoXmlNS::text, "start-value", QString::null ).toInt();
            bool isListHeader = listItem.localName() == "list-header" || listItem.attributeNS( KoXmlNS::text, "is-list-header", QString::null ) == "is-list-header";
            KoTextParag* oldLast = lastParagraph;
            lastParagraph = loadOasisText( listItem, context, lastParagraph, styleColl, nextParagraph );
            KoTextParag* firstListItem = oldLast ? oldLast->next() : firstParag();
            KoTextParag* p = firstListItem;
            // It's either list-header (normal text on top of list) or list-item
            if ( !isListHeader && firstListItem ) {
                // Apply list style to first paragraph inside list-item
                bool isOutline = firstListItem->counter() && firstListItem->counter()->numbering() == KoParagCounter::NUM_CHAPTER;
                firstListItem->applyListStyle( context, restartNumbering, orderedList, isOutline, level );
                p = p->next();
            }
            // Make text:h inside list-item (as non first child) unnumbered.
            while ( p && p != lastParagraph->next() ) {
                if ( p->counter() ) p->counter()->setNumbering( KoParagCounter::NUM_NONE );
                p = p->next();
            }
        }
    }
    if ( listOK ) context.listStyleStack().pop();
    context.setCurrentListStyleName( oldListStyleName );
    return lastParagraph;
#else

    // Get the name of the used style
    QString styleName;
    if ( parent.hasAttributeNS( KoXmlNS::text, "style-name" ) ) {
        styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString::null );
        //KoXmlElement* listElem = context.oasisStyles().listStyles()[ styleName ];
        //if(listElem) context.addStyles( listElem, "paragraph" );
    }

    // Get the KoListStyle the name may reference to
    KoListStyle* listStyle = d->listStyle(styleName);
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadList styleName="<<styleName<<" listStyle="<<(listStyle ? listStyle->name() : "NULL")<<endl;
    #endif
    if( ! listStyle ) { // even if wellformed documents shouldn't reference a style we don't know about yet, we handle that case...
        listStyle = new KoListStyle();
        listStyle->setName(styleName);
        d->addStyle(listStyle);
    }


KoParagraphStyle* paragStyle = new KoParagraphStyle();
paragStyle->setName(styleName);
paragStyle->setListStyle(*listStyle);





    // Set the style and create the textlist
    QTextListFormat listformat;
    QTextList* list = cursor.insertList(listformat);

    // we need at least one item, so add a dummy-item we remove later again
    cursor.insertBlock();
    QTextBlock prev = cursor.block();
static int level = 1;
    // Iterate over list items and add them to the textlist
    KoXmlElement e;
    forEachElement(e, parent) {
        if( e.isNull() ) continue;
        /*
        //TODO handle also the other item properties
        if( e.hasAttributeNS( KoXmlNS::text, "start-value" ) ) {
            int startValue = e.attributeNS(KoXmlNS::text, "start-value", QString::null).toInt();
            KoListLevelProperties p = KoListLevelProperties::fromTextList(list);
            p.setStartValue(startValue);
            QTextListFormat f = list->format();
            p.applyStyle(f);
            list->setFormat(f);
        }
        */

//KoListLevelProperties properties = listStyle->level(level);
//QTextListFormat lf;
//properties.applyStyle(lf);
//list->setFormat(lf);

QTextBlock bl = cursor.block();
paragStyle->applyStyle(bl);
paragStyle->setListLevel(level);
        //listStyle->applyStyle(cursor.block(), level);
        loadBody(context, e, cursor);
level++;
    }

    // add the new blocks to the list
    QTextBlock current = cursor.block();
    for(QTextBlock b = prev; b.isValid() && b != current; b = b.next())
        list->add(b);
    list->removeItem(0); // remove the first dummy item again




    /*
    // Get the matching paragraph style
    //QString userStyleName = context.styleStack().userStyleName( "paragraph" );
    KoParagraphStyle *paragStyle = d->paragraphStyle(styleName);
    if( ! paragStyle ) {
        //paragStyle = d->stylemanager->defaultParagraphStyle();
        paragStyle = new KoParagraphStyle();
        paragStyle->setName(styleName);
        d->addStyle(paragStyle);
        context.styleStack().setTypeProperties( "paragraph" ); // load all style attributes from "style:paragraph-properties"
        paragStyle->loadOasis(context.styleStack()); // load the KoParagraphStyle from the stylestack
        KoCharacterStyle *charstyle = paragStyle->characterStyle();
        context.styleStack().setTypeProperties( "text" ); // load all style attributes from "style:text-properties"
        charstyle->loadOasis(context); // load the KoCharacterStyle from the stylestack
    }
    //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );

    // The paragraph style has a list style
    KoListStyle prevliststyle = paragStyle->listStyle();
    KoListStyle* listStyle = prevliststyle.isValid() ? new KoListStyle(prevliststyle) : new KoListStyle();
    listStyle->setName(styleName);
    listStyle->loadOasis(context);
    paragStyle->setListStyle(*listStyle);
    */

    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);
#endif
}

//1.6: KoTextDocument::loadOasisText
void KoOpenDocumentLoader::loadSection(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug()<<"KoOpenDocumentLoader::loadSection"<<endl;
    #endif

    //TODO
    //kdDebug(32500) << "Section found!" << endl;
    //context.fillStyleStack( tag, KoXmlNS::text, "style-name", "section" );
    //lastParagraph = loadOasisText( tag, context, lastParagraph, styleColl, nextParagraph );
    Q_UNUSED(context);
    Q_UNUSED(parent);
    Q_UNUSED(cursor);
}

// we cannot use QString::simplifyWhiteSpace() because it removes
// leading and trailing whitespace, but such whitespace is significant
// in ODF -- so we use this function to compress sequences of space characters
// into single spaces
static QString normalizeWhitespace( const QString& in, bool leadingSpace )
{
    QString text = in;
    int r, w = 0;
    int len = text.length();
    for ( r = 0; r < len; ++r ) {
        QCharRef ch = text[r];
        // check for space, tab, line feed, carriage return
        if ( ch == ' ' || ch == '\t' ||ch == '\r' ||  ch == '\n') {
            // if we were lead by whitespace in some parent or previous sibling element,
            // we completely collapse this space
            if ( r != 0 || !leadingSpace )
                text[w++] = QChar( ' ' );
            // find the end of the whitespace run
            while ( r < len && text[r].isSpace() )
                ++r;
            // and then record the next non-whitespace character
            if ( r < len )
               text[w++] = text[r];
        }
        else {
            text[w++] = ch;
        }
    }
    // and now trim off the unused part of the string
    text.truncate(w);
    return text;
}

#if 0
// First loadFrame test
void KoOpenDocumentLoader::loadFrame(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    float width, height;
    QDomNamedNodeMap attrs = parent.attributes();
    for (int iAttr = 0 ; iAttr < attrs.count() ; iAttr++) {
        kDebug() << "Attribute " << iAttr << " : " << attrs.item(iAttr).nodeName() << "\t" << attrs.item(iAttr).nodeValue() << endl;
        if (attrs.item(iAttr).nodeName() == "svg:width") {
            width = KoUnit::parseValue(attrs.item(iAttr).nodeValue());
        } else if (attrs.item(iAttr).nodeName() == "svg:height") {
            height = KoUnit::parseValue(attrs.item(iAttr).nodeValue());
        }
    }
    for (KoXmlNode node = parent.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        KoXmlElement ts = node.toElement();
        const QString localName( ts.localName() );
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;
        if (isDrawNS && localName == "image") {
            attrs = ts.attributes();
            QString href;
            for (int iAttr = 0 ; iAttr < attrs.count() ; iAttr++) {
                kDebug() << "Attribute " << iAttr << " : " << attrs.item(iAttr).nodeName() << "\t" << attrs.item(iAttr).nodeValue() << endl;
                if (attrs.item(iAttr).localName() == "href") href = attrs.item(iAttr).nodeValue();
            }
            if (context.store()->hasFile(href)) {
                kDebug() << "Ok, picture available in the store" << endl;
                if (context.store()->isOpen()) {
                    kDebug() << "Shit, store already reading something" << endl;
                } else {
                    kDebug() << "Ok, I can handle it" << endl;
                    if (context.store()->open(href)) {
                        kDebug() << "Great, it's opened now" << endl;
                        QImage img;
                        if (img.load(context.store()->device(), "png")) {
                            kDebug() << "Image1 : " << img.size() << endl;
                            d->document->mainFrameSet()->document()->addResource(QTextDocument::ImageResource, href, img);
                            /*kDebug() << d->document->mainFrameSet()->document()->resource(QTextDocument::ImageResource, href) << endl;
                            QImage test = d->document->mainFrameSet()->document()->resource(QTextDocument::ImageResource, href).value<QImage>();
                            kDebug() << "Image2 : " << test.size() << endl;*/
                            cursor.insertImage(href);
                        } else {
                            kDebug() << "SHIT" << endl;
                        }
                        context.store()->close();
                    }
                }
            } else kDebug() << "Sad, picture not available..." << endl;
        } else kDebug() << "Sorry kid, this isn't handled currently" << endl;
    }
}
#endif

//1.6: KoTextParag::loadOasisSpan
void KoOpenDocumentLoader::loadSpan(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor, bool* stripLeadingSpace)
{
    Q_ASSERT( stripLeadingSpace );
    for (KoXmlNode node = parent.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        KoXmlElement ts = node.toElement();
        const QString localName( ts.localName() );
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        //const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;

        // allow loadSpanTag to modify the stylestack
        context.styleStack().save();

        if ( node.isText() )
        {
            QString text = node.toText().data();
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug() << "  <text> localName=" << localName << " parent.localName="<<parent.localName()<<" text=" << text << endl;
            #endif
            text = normalizeWhitespace(text.replace('\n', QChar(0x2028)), *stripLeadingSpace);

            if ( text.isEmpty() )
                *stripLeadingSpace = false;
            else
                *stripLeadingSpace = text[text.length() - 1].isSpace();

            cursor.insertText( text );
        }
        else if ( isTextNS && localName == "span" ) // text:span
        {
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug() << "  <span> localName=" << localName << endl;
            #endif
            context.fillStyleStack( ts, KoXmlNS::text, "style-name", "text" );
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            context.styleStack().setTypeProperties( "text" );
            const QString textStyleName = ts.attributeNS( KoXmlNS::text, "style-name", QString() );
            const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.oasisStyles().findStyle( textStyleName, "text" );
            KoCharacterStyle *charstyle = 0;
            if( textStyleElem ) {
                #ifdef KOOPENDOCUMENTLOADER_DEBUG
                    kDebug()<<"textStyleName="<<textStyleName<<endl;
                #endif
                context.addStyles( textStyleElem, "text" );
                charstyle = d->characterStyle(textStyleName);
                if( ! charstyle ) {
                    charstyle = new KoCharacterStyle();
                    charstyle->setName(textStyleName);
                    charstyle->loadOasis(context);
                    d->addStyle(charstyle);
                }
                charstyle->applyStyle(&cursor);
            }

            loadSpan(context, ts, cursor, stripLeadingSpace); // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
        }
        else if ( isTextNS && localName == "s" ) // text:s
        {
            int howmany = 1;
            if (ts.hasAttributeNS( KoXmlNS::text, "c"))
                howmany = ts.attributeNS( KoXmlNS::text, "c", QString()).toInt();
            cursor.insertText( QString().fill(32, howmany) );
        }
        else if ( isTextNS && localName == "tab" ) // text:tab
        {
            cursor.insertText( "\t" );
        }
        else if ( isTextNS && localName == "line-break" ) // text:line-break
        {
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug() << "  <line-break> Node localName=" << localName << endl;
            #endif
            cursor.insertText( "\n" );
        }
        else if ( isTextNS && localName == "number" ) // text:number
        {
            // This is the number in front of a numbered paragraph,
            // written out to help export filters. We can ignore it.
        }
#if 0 // Load Frame test disabled
        else if ( isDrawNS && localName == "frame" ) // draw:frame
        {
            // We are opening a new frame...
            loadFrame(ts, cursor, stripLeadingSpace);
        }
#endif
        else
        {
#if 0 //1.6:
            bool handled = false;
            // Check if it's a variable
            KoVariable* var = context.variableCollection().loadOasisField( textDocument(), ts, context );
            if ( var ) {
                textData = "#";     // field placeholder
                customItem = var;
                handled = true;
            }
            if ( !handled ) {
                handled = textDocument()->loadSpanTag( ts, context, this, pos, textData, customItem );
                if ( !handled ) {
                    kWarning(32500) << "Ignoring tag " << ts.tagName() << endl;
                    context.styleStack().restore();
                    continue;
                }
            }
#else
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug() << "  Node localName=" << localName << " is UNHANDLED" << endl;
            #endif
#endif
        }

        // restore the propably by loadSpanTag modified stylestack
        context.styleStack().restore();
    }
}

#include "KoOpenDocumentLoader.moc"
