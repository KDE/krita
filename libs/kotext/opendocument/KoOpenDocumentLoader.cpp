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

/// \internal d-pointer class.
class KoOpenDocumentLoader::Private
{
    public:
        KoStyleManager* stylemanager;
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
#if 0 //1.6:
    QStringList followingStyles;
    QList<KoXmlElement*> userStyles = context.oasisStyles().customStyles( "paragraph" ).values();
    bool defaultStyleDeleted = false;
    int stylesLoaded = 0;
    const unsigned int nStyles = userStyles.count();
    for (unsigned int item = 0; item < nStyles; item++) {
        KoXmlElement* styleElem = userStyles[item];
        if ( !styleElem ) continue;
        Q_ASSERT( !styleElem->isNull() );
        if( !defaultStyleDeleted ) { // we are going to import at least one style.
            KoParagStyle *s = defaultStyle();
            //kDebug() << "loadOasisStyles looking for Standard, to delete it. Found " << s << endl;
            if(s) removeStyle(s); // delete the standard style.
            defaultStyleDeleted = true;
        }
        KoParagStyle *sty = new KoParagStyle( QString::null );
        // Load the style
        sty->loadStyle( *styleElem, context );
        // Style created, now let's try to add it
        const int oldStyleCount = count();
        sty = addStyle( sty );
        // the real value of followingStyle is set below after loading all styles
        sty->setFollowingStyle( sty );
        kDebug() << " Loaded style " << sty->name() << endl;
        if ( count() > oldStyleCount ) {
            const QString following = styleElem->attributeNS( KoXmlNS::style, "next-style-name", QString::null );
            followingStyles.append( following );
            ++stylesLoaded;
        }
        else kWarning() << "Found duplicate style declaration, overwriting former " << sty->name() << endl;
    }
    if( followingStyles.count() != styleList().count() ) kDebug() << "Ouch, " << followingStyles.count() << " following-styles, but " << styleList().count() << " styles in styleList" << endl;
    unsigned int i = 0;
    QString tmpString;
    foreach( tmpString, followingStyles ) {
        const QString followingStyleName = tmpString;
        if ( !followingStyleName.isEmpty() ) {
            KoParagStyle * style = findStyle( followingStyleName );
            if ( style ) styleAt(i)->setFollowingStyle( style );
        }
    }
    // TODO the same thing for style inheritance (style:parent-style-name) and setParentStyle()
    Q_ASSERT( defaultStyle() );
    return stylesLoaded;
#endif
    foreach(KoXmlElement* styleElem, styleElements) {
        Q_ASSERT( styleElem );
        Q_ASSERT( !styleElem->isNull() );

        //1.6: KoParagStyle::loadStyle
        QString name = styleElem->attributeNS( KoXmlNS::style, "name", QString() );
        QString displayName = styleElem->attributeNS( KoXmlNS::style, "display-name", QString() );
        if ( displayName.isEmpty() )
            displayName = name;

        kDebug(32001)<<"KoOpenDocumentLoader::loadStyles styleName="<<name<<" styleDisplayName="<<displayName<<endl;
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
        d->stylemanager->add(parastyle);

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
    kDebug(32001)<<"KoOpenDocumentLoader::loadAllStyles"<<endl;
    // User styles are named and appear in the gui while automatic styles are just a way to
    // save formatting changes done by the user. There is no real tech diff between them
    // except how we present them to the user.
    loadStyles(context, context.oasisStyles().autoStyles("paragraph").values());
    loadStyles(context, context.oasisStyles().customStyles("paragraph").values());
}

void KoOpenDocumentLoader::loadSettings(KoOasisLoadingContext& context, const QDomDocument& settings)
{
    kDebug(32001)<<"KoOpenDocumentLoader::loadSettings"<<endl;
    Q_UNUSED(context);
    Q_UNUSED(settings);
}

bool KoOpenDocumentLoader::loadPageLayout(KoOasisLoadingContext& context, const QString& masterPageName)
{
    kDebug(32001)<<"KoOpenDocumentLoader::loadPageLayout"<<endl;
    Q_UNUSED(context);
    Q_UNUSED(masterPageName);
    return true;
}

bool KoOpenDocumentLoader::loadMasterPageStyle(KoOasisLoadingContext& context, const QString& masterPageName)
{
    kDebug(32001)<<"KoOpenDocumentLoader::loadMasterPageStyle"<<endl;
    Q_UNUSED(context);
    Q_UNUSED(masterPageName);
    return true;
}

//1.6: KoTextDocument::loadOasisText
void KoOpenDocumentLoader::loadBody(KoOasisLoadingContext& context, const KoXmlElement& bodyElem, QTextCursor& cursor)
{
    kDebug(32001)<<"KoOpenDocumentLoader::loadBody"<<endl;
    KoXmlElement tag;
    forEachElement(tag, bodyElem) {
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
        //else kDebug(32001)<<"Unhandled localName="<<localName<<endl;

        context.styleStack().restore(); // remove the styles added by the paragraph or list
    }
}

//1.6: KoTextDocument::loadOasisText
void KoOpenDocumentLoader::loadParagraph(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );
    QString userStyleName = context.styleStack().userStyleName( "paragraph" );
    kDebug(32001)<<"KoOpenDocumentLoader::loadParagraph userStyleName="<<userStyleName<<endl;
    KoParagraphStyle *userStyle = d->stylemanager->paragraphStyle(userStyleName);
    //if( ! userStyle ) userStyle = d->stylemanager->defaultParagraphStyle();
    if( userStyle ) {
        context.styleStack().setTypeProperties( "paragraph" );
        //1.6: KoTextParag::loadOasis( tag, context, styleColl, pos )
        userStyle->loadOasis( context.styleStack() ); //FIXME don't reload each time
        QTextBlock block = cursor.block();
        userStyle->applyStyle(block);
    }

    //1.6: KWTextParag::loadOasis
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    kDebug()<<"==> PARAGRAPH styleName="<<styleName<<" userStyleName="<<userStyleName<<" userStyle="<<(userStyle?"YES":"NULL")<<endl;
    if ( !styleName.isEmpty() ) {
        const QDomElement* paragraphStyle = context.oasisStyles().findStyle( styleName, "paragraph" );
        QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString() ) : QString();
        if ( masterPageName.isEmpty() )
            masterPageName = "Standard";
        kDebug(32001) << "KoOpenDocumentLoader::loadParagraph paragraphStyle.localName=" << (paragraphStyle ? paragraphStyle->localName() : "NULL") << " masterPageName=" << masterPageName << endl;

        /*
        QString styleName = context.styleStack().userStyleName( "paragraph" );
        KoParagraphStyle *style = d->stylemanager->paragraphStyle(styleName);
        if ( !style ) {
            kDebug(32001) << "KoOpenDocumentLoader::loadSpan: Unknown style. Using default!" << endl;
            style = d->stylemanager->defaultParagraphStyle();
        }
        */

        //d->currentMasterPage = masterPageName; // do this first to avoid recursion
        context.styleStack().save();
        context.styleStack().setTypeProperties( "paragraph" );
        if( paragraphStyle )
            context.addStyles( paragraphStyle, "paragraph" );
        //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );

        KoParagraphStyle *style = d->stylemanager->paragraphStyle(styleName);
        if( ! style ) {
            style = d->stylemanager->defaultParagraphStyle();
            kDebug(32001) << "KoOpenDocumentLoader::loadParagraph using default style!" << endl;
        }
        if ( style ) {
            style->loadOasis( context.styleStack() );
            QTextBlock block = cursor.block();
            style->applyStyle(block);
        }

#if 0
        // This is quite ugly... OOo stores the starting page-number in the first paragraph style...
        QString pageNumber = context.styleStack().attributeNS( KoXmlNS::style, "page-number" );
        if ( !pageNumber.isEmpty() ) doc->variableCollection()->variableSetting()->setStartingPageNumber( pageNumber.toInt() );
#endif
        context.styleStack().restore();

//is this needed here at all?
loadPageLayout(context, masterPageName); // page layout
    }

    //KoTextParag::loadOasisSpan
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "text" );
    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    context.styleStack().setTypeProperties( "paragraph" );
    const QString textStyleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.oasisStyles().findStyle( textStyleName, "paragraph" );
    KoCharacterStyle *charstyle = 0;
    if( textStyleElem ) {
        context.addStyles( textStyleElem, "paragraph" );
        charstyle = d->stylemanager->characterStyle(textStyleName);
        if( ! charstyle ) {
            charstyle = new KoCharacterStyle();
            charstyle->setName(textStyleName);
            charstyle->loadOasis(context);
            d->stylemanager->add(charstyle);
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
    kDebug(32001)<<"KoOpenDocumentLoader::loadHeading"<<endl;
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
    //1.6: KWTextParag::loadOasis
    const QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    kDebug()<<"==> HEADING styleName="<<styleName<<endl;
    if ( !styleName.isEmpty() ) {
        const QDomElement* paragraphStyle = context.oasisStyles().findStyle( styleName, "paragraph" );
        //QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString::null ) : QString::null;
        //if ( masterPageName.isEmpty() ) masterPageName = "Standard"; // Seems to be a builtin name for the default layout...
        kDebug(32001) << "KoOpenDocumentLoader::loadBody styleName=" << styleName << endl;
        context.styleStack().save();
        context.styleStack().setTypeProperties( "paragraph" );
        if( paragraphStyle )
            context.addStyles( paragraphStyle, "paragraph" );
        context.styleStack().restore();
        //loadPageLayout( masterPageName, context ); // page layout

        KoParagraphStyle *style = d->stylemanager->paragraphStyle(styleName);
        if ( style ) {
            style->loadOasis( context.styleStack() );
            QTextBlock block = cursor.block();
            style->applyStyle(block);
        }
    }
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
    kDebug(32001)<<"KoOpenDocumentLoader::loadList"<<endl;
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
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );

    QString styleName;
    if( parent.hasAttributeNS( KoXmlNS::text, "style-name" ) )
        styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );

    //QString userStyleName = context.styleStack().userStyleName( "paragraph" );
    KoParagraphStyle *paragstyle = d->stylemanager->paragraphStyle(styleName);
    if( ! paragstyle ) {
        //paragstyle = d->stylemanager->defaultParagraphStyle();
        paragstyle = new KoParagraphStyle();
        paragstyle->setName(styleName);
        d->stylemanager->add(paragstyle);
        context.styleStack().setTypeProperties( "paragraph" ); // load all style attributes from "style:paragraph-properties"
        paragstyle->loadOasis(context.styleStack()); // load the KoParagraphStyle from the stylestack
        KoCharacterStyle *charstyle = paragstyle->characterStyle();
        context.styleStack().setTypeProperties( "text" ); // load all style attributes from "style:text-properties"
        charstyle->loadOasis(context); // load the KoCharacterStyle from the stylestack
    }
    //context.styleStack().setTypeProperties( "paragraph" );
    //style->loadOasis( context.styleStack() );

    KoListStyle *liststyle =  new KoListStyle();
    liststyle->loadOasis(context.styleStack());
    paragstyle->setListStyle(*liststyle);

    //QTextBlockFormat emptyTbf1;
    //QTextCharFormat emptyCf1;
    //cursor.insertBlock(emptyTbf1, emptyCf1);

    //TESTCASE
    QTextListFormat listformat;
    //listformat.setIndent(2);
    listformat.setStyle( QTextListFormat::ListDisc );
    QTextList* list = cursor.insertList(listformat);

    // Iterate over list items
    for(QDomNode n = parent.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if( e.isNull() ) continue;
        cursor.insertBlock();
        QTextBlock prev = cursor.block();
        loadBody(context, e, cursor);
        QTextBlock current = cursor.block();
        //TODO merge all blocks added by the item to apply the style on all of them
        for(QTextBlock b = prev; b.isValid() && b != current; b = b.next())
            list->add(b);
        //list->add( cursor.block() );
    }
    //delete liststyle;

    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);
#endif
}

//1.6: KoTextDocument::loadOasisText
void KoOpenDocumentLoader::loadSection(KoOasisLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    kDebug(32001)<<"KoOpenDocumentLoader::loadSection"<<endl;
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
            kDebug() << "  <text> localName=" << localName << " parent.localName="<<parent.localName()<<" text=" << text << endl;
            text = normalizeWhitespace(text.replace('\n', QChar(0x2028)), *stripLeadingSpace);

            if ( text.isEmpty() )
                *stripLeadingSpace = false;
            else
                *stripLeadingSpace = text[text.length() - 1].isSpace();

            cursor.insertText( text );
        }
        else if ( isTextNS && localName == "span" ) // text:span
        {
            kDebug() << "  <span> localName=" << localName << endl;
            context.fillStyleStack( ts, KoXmlNS::text, "style-name", "text" );
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            context.styleStack().setTypeProperties( "text" );
            const QString textStyleName = ts.attributeNS( KoXmlNS::text, "style-name", QString() );
            const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.oasisStyles().findStyle( textStyleName, "text" );
            KoCharacterStyle *charstyle = 0;
            if( textStyleElem ) {
                kDebug()<<"textStyleName="<<textStyleName<<endl;
                context.addStyles( textStyleElem, "text" );
                charstyle = d->stylemanager->characterStyle(textStyleName);
                if( ! charstyle ) {
                    charstyle = new KoCharacterStyle();
                    charstyle->setName(textStyleName);
                    charstyle->loadOasis(context);
                    d->stylemanager->add(charstyle);
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
            kDebug() << "  <line-break> Node localName=" << localName << endl;
            QTextBlockFormat emptyTbf;
            QTextCharFormat emptyCf;
            cursor.insertBlock(emptyTbf, emptyCf);
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
            kDebug() << "  Node localName=" << localName << " is UNHANDLED" << endl;
#endif
        }

        // restore the propably by loadSpanTag modified stylestack
        context.styleStack().restore();
    }
}
//#endif

#include "KoOpenDocumentLoader.moc"
