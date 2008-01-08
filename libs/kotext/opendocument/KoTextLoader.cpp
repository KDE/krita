/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoTextLoader.h"
#include "KoTextFrameLoader.h"

// koffice
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoOasisLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoImageData.h>
#include <KoTextAnchor.h>
#include <KoTextDocumentLayout.h>
#include <KoVariableManager.h>
#include <KoInlineTextObjectManager.h>
#include <KoInlineObjectRegistry.h>
#include <KoProperties.h>

#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextSharedLoadingData.h"

// KDE + Qt includes
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <QTime>
#include <klocale.h>
#include <kdebug.h>

// if defined then debugging is enabled
#define KOOPENDOCUMENTLOADER_DEBUG

/// \internal d-pointer class.
class KoTextLoader::Private
{
    public:
        KoShapeLoadingContext & context;
        KoTextSharedLoadingData * textSharedData;
        // store it here so that you don't need to get it all the time from 
        // the KoOasisLoadingContext.
        bool stylesDotXml;

        int bodyProgressTotal;
        int bodyProgressValue;
        int lastElapsed;
        QTime dt;

        QString currentListStyleName;
        int currentListLevel;

        explicit Private( KoShapeLoadingContext & context )
        : context( context )
        , textSharedData( 0 )
        , stylesDotXml( context.koLoadingContext().useStylesAutoStyles() )
        , bodyProgressTotal( 0 )
        , bodyProgressValue( 0 )
        , lastElapsed( 0 )
        , currentListLevel( 1 )
        {
            dt.start();
        }

        ~Private()
        {
            kDebug(32500) <<"Loading took" << (float)(dt.elapsed()) / 1000 <<" seconds";
        }

};

KoTextLoader::KoTextLoader( KoShapeLoadingContext & context )
: QObject()
, d( new Private( context ) )
{
    KoSharedLoadingData * sharedData = context.sharedData( KOTEXT_SHARED_LOADING_ID );

    if ( sharedData ) {
        d->textSharedData = dynamic_cast<KoTextSharedLoadingData *>( sharedData );
    }

    kDebug(32500) << "sharedData" << sharedData << "textSharedData" << d->textSharedData;

    if ( !d->textSharedData ) {
        d->textSharedData = new KoTextSharedLoadingData();
        // TODO pass style manager so that on copy and paste we can recognice the same styles
        d->textSharedData->loadOdfStyles( context.koLoadingContext(), 0 );
        if ( !sharedData ) {
            context.addSharedData( KOTEXT_SHARED_LOADING_ID, d->textSharedData );
        }
        else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_LOADING_ID;
            Q_ASSERT( false );
        }
    }
}

KoTextLoader::~KoTextLoader()
{
    delete d;
}

#if 0
//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadBody(KoTextLoadingContext& context, const KoXmlElement& bodyElem, QTextCursor& cursor)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"KoTextLoader::loadBody";
    #endif

    startBody( KoXml::childNodesCount( bodyElem ) );
    KoXmlElement tag;
    forEachElement(tag, bodyElem) {
        if( ! tag.isNull() ) {
            context.styleStack().save();
            const QString localName = tag.localName();
            if( tag.namespaceURI() == KoXmlNS::text ) {
                if ( localName == "p" ) {  // text paragraph
                    loadParagraph(context, tag, cursor);
                }
                else if ( localName == "h" ) { // heading
                    loadHeading(context, tag, cursor);
                }
                else if ( localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                            || localName == "list" || localName == "numbered-paragraph" ) { // OASIS
                    loadList(context, tag, cursor);
                }
                else if ( localName == "section" ) { // Temporary support (###TODO)
                    loadSection(context, tag, cursor);
                }
                else {
                    kWarning(32500)<<"KoTextLoader::loadBody unhandled text::"<<localName;
                }
            }
            else if( tag.namespaceURI() == KoXmlNS::draw ) {
                if ( localName == "frame" ) {
                    loadFrame(context, tag, cursor);
                }
                else {
                    kWarning(32500)<<"KoTextLoader::loadBody unhandled draw::"<<localName;
                }
            } else if( tag.namespaceURI() == KoXmlNS::table ) {
                if ( localName == "table" ) {
                    loadFrame(context, tag, cursor);
#if 0 // Loading is done by the table shape
                    cursor.insertText("\n");
                    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
                    QTextTable *tbl = cursor.insertTable(1, 1);
                    int rows = 0;
                    int columns = 0;
                    kDebug(32500) <<"Table inserted";
                    KoXmlElement tblTag;
                    forEachElement(tblTag, tag) {
                        if( ! tblTag.isNull() ) {
                            const QString tblLocalName = tblTag.localName();
                            if (tblTag.namespaceURI() == KoXmlNS::table) {
                                if (tblLocalName == "table-column") {
                                    // Do some parsing with the column, see §8.2.1, ODF 1.1 spec
                                    int repeatColumn = tblTag.attributeNS(KoXmlNS::table, "number-columns-repeated", "1").toInt();
                                    columns = columns + repeatColumn;
                                    if (rows > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(1, columns);
                                } else if (tblLocalName == "table-row") {
                                    // Lot of work to do here...
                                    rows++;
                                    if (columns > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(rows, 1);
                                    // Added a row
                                    int currentCell = 0;
                                    KoXmlElement rowTag;
                                    forEachElement(rowTag, tblTag) {
                                        if (!rowTag.isNull()) {
                                            const QString rowLocalName = rowTag.localName();
                                            if (rowTag.namespaceURI() == KoXmlNS::table) {
                                                if (rowLocalName == "table-cell") {
                                                    // Ok, it's a cell...
                                                    const int currentRow = tbl->rows() - 1;
                                                    QTextTableCell cell = tbl->cellAt(currentRow, currentCell);
                                                    if (cell.isValid()) {
                                                        cursor = cell.firstCursorPosition();
                                                        loadBody(context, rowTag, cursor);
                                                    }
                                                    else
                                                        kDebug(32500)<<"Invalid table-cell row="<<currentRow<<" column="<<currentCell;
                                                    currentCell++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    cursor = tbl->lastCursorPosition ();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
#endif                    
                }
                else {
                    kWarning(32500)<<"KoTextLoader::loadBody unhandled table::"<<localName;
                }
            }
            context.styleStack().restore(); // remove the styles added by the paragraph or list
        }
        processBody();
    }
    endBody();
}
#endif

void KoTextLoader::loadBody( const KoXmlElement& bodyElem, QTextCursor& cursor )
{
    kDebug(32500) << "";

    startBody( KoXml::childNodesCount( bodyElem ) );
    KoXmlElement tag;
    forEachElement(tag, bodyElem) {
        if ( ! tag.isNull() ) {
            const QString localName = tag.localName();
            if ( tag.namespaceURI() == KoXmlNS::text ) {
                if ( localName == "p" ) {  // text paragraph
                    loadParagraph( tag, cursor );
                }
                else if ( localName == "h" ) { // heading
                    loadHeading( tag, cursor );
                }
                else if ( localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                            || localName == "list" || localName == "numbered-paragraph" ) { // OASIS
                    loadList( tag, cursor );
                }
                else if ( localName == "section" ) { // Temporary support (###TODO)
                    loadSection( tag, cursor );
                }
                else {
                    kWarning(32500) << "unhandled text:" << localName;
                }
            }
            else if( tag.namespaceURI() == KoXmlNS::draw ) {
                if ( localName == "frame" ) {
                    loadFrame( tag, cursor );
                }
                else {
                    kWarning(32500) << "unhandled draw:" << localName;
                }
            } else if( tag.namespaceURI() == KoXmlNS::table ) {
#if 0 // TODO commented out for now
                if ( localName == "table" ) {
                    cursor.insertText("\n");
                    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
                    QTextTable *tbl = cursor.insertTable(1, 1);
                    int rows = 0;
                    int columns = 0;
                    kDebug(32500) <<"Table inserted";
                    KoXmlElement tblTag;
                    forEachElement(tblTag, tag) {
                        if( ! tblTag.isNull() ) {
                            const QString tblLocalName = tblTag.localName();
                            if (tblTag.namespaceURI() == KoXmlNS::table) {
                                if (tblLocalName == "table-column") {
                                    // Do some parsing with the column, see §8.2.1, ODF 1.1 spec
                                    int repeatColumn = tblTag.attributeNS(KoXmlNS::table, "number-columns-repeated", "1").toInt();
                                    columns = columns + repeatColumn;
                                    if (rows > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(1, columns);
                                } else if (tblLocalName == "table-row") {
                                    // Lot of work to do here...
                                    rows++;
                                    if (columns > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(rows, 1);
                                    // Added a row
                                    int currentCell = 0;
                                    KoXmlElement rowTag;
                                    forEachElement(rowTag, tblTag) {
                                        if (!rowTag.isNull()) {
                                            const QString rowLocalName = rowTag.localName();
                                            if (rowTag.namespaceURI() == KoXmlNS::table) {
                                                if (rowLocalName == "table-cell") {
                                                    // Ok, it's a cell...
                                                    const int currentRow = tbl->rows() - 1;
                                                    QTextTableCell cell = tbl->cellAt(currentRow, currentCell);
                                                    if (cell.isValid()) {
                                                        cursor = cell.firstCursorPosition();
                                                        loadBody(context, rowTag, cursor);
                                                    }
                                                    else
                                                        kDebug(32500)<<"Invalid table-cell row="<<currentRow<<" column="<<currentCell;
                                                    currentCell++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    cursor = tbl->lastCursorPosition ();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
                }
                else {
                    kWarning(32500)<<"KoTextLoader::loadBody unhandled table::"<<localName;
                }
#endif
            }
        }
        processBody();
    }
    endBody();
}

#if 0
//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadParagraph(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );
    QString userStyleName = context.styleStack().userStyleName( "paragraph" );
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"userStyleName="<<userStyleName;
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
        kDebug(32500)<<"styleName="<<styleName<<" userStyleName="<<userStyleName<<" userStyle="<<(userStyle?"YES":"NULL");
    #endif
    if ( !styleName.isEmpty() ) {
        const KoXmlElement* paragraphStyle = context.stylesReader().findStyle( styleName, "paragraph" );
        QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString() ) : QString();
        if ( masterPageName.isEmpty() )
            masterPageName = "Standard";
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) <<"paragraphStyle.localName=" << (paragraphStyle ? paragraphStyle->localName() :"NULL") <<" masterPageName=" << masterPageName;
        #endif

        /*
        QString styleName = context.styleStack().userStyleName( "paragraph" );
        KoParagraphStyle *style = d->stylemanager->paragraphStyle(styleName);
        if ( !style ) {
            kDebug(32500) <<"KoTextLoader::loadSpan: Unknown style. Using default!";
            style = d->stylemanager->defaultParagraphStyle();
        }
        */

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
                kDebug(32500) <<"using default style!";
            #endif
        }
        if ( style ) {
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
//loadPageLayout(context, masterPageName); // page layout
    }

    //KoTextParag::loadOasisSpan
    //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "text" ); //is this really needed?

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
    context.styleStack().setTypeProperties( "paragraph" );
    const QString textStyleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.stylesReader().findStyle( textStyleName, "paragraph" );
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
#endif

void KoTextLoader::loadParagraph( const KoXmlElement& element, QTextCursor& cursor )
{
    // TODO use the default style name a default value?
    QString styleName = element.attributeNS( KoXmlNS::text, "style-name", QString() );

    KoParagraphStyle * paragraphStyle = d->textSharedData->paragraphStyle( styleName, d->stylesDotXml );

    if ( paragraphStyle ) {
        QTextBlock block = cursor.block();
        paragraphStyle->applyStyle( block );
    }
    else {
        kWarning(32500) << "paragraph style " << styleName << " not found";
    }

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    bool stripLeadingSpace = true;
    loadSpan( element, cursor, &stripLeadingSpace );
    cursor.setCharFormat( cf ); // restore the cursor char format

    // tz: why is this done
    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);
}

#if 0
//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadHeading(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );
#if 0 //1.6:
    int level = tag.attributeNS( KoXmlNS::text, "outline-level", QString() ).toInt();
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
        restartNumbering = tag.attributeNS( KoXmlNS::text, "start-value", QString() ).toInt();
    KoTextParag *parag = createParag( this, lastParagraph, nextParagraph );
    parag->loadOasis( tag, context, styleColl, pos );
    if ( !lastParagraph ) setFirstParag( parag ); // First parag
    lastParagraph = parag;
    if ( listOK ) {
        parag->applyListStyle( context, restartNumbering, true /*ordered*/, true /*heading*/, level );
        context.listStyleStack().pop();
    }
#endif
    int level = parent.attributeNS( KoXmlNS::text, "outline-level", QString() ).toInt();
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );

    // Get the KoListStyle the name may reference to
    KoListStyle* listStyle = d->listStyle(styleName);
    if( ! listStyle ) { // no such list means we define a new one one
        listStyle = new KoListStyle();
        listStyle->setName(styleName);
        d->addStyle(listStyle);
    }
    //context.setCurrentListStyleName(styleName);
    //int level = context.currentListLevel();

    kDebug(32500)<<"parent.localName="<<parent.localName()<<"style-name="<<styleName<<" outline-level="<<level;

    // Each header is within a list. That allows us to have them numbered on demand.
    QTextListFormat listformat;
    QTextList* list = cursor.createList(listformat);

    /*
    //1.6: KoOasisContext::pushOutlineListLevelStyle
    //KoXmlElement outlineStyle = KoDom::namedItemNS( stylesReader().officeStyle(), KoXmlNS::text, "outline-style" );
    KoListStyle* listStyle = 0;
    if( level > 0 ) {
        listStyle = new KoListStyle();
        KoListLevelProperties props;
        //props.setListItemPrefix("ABC");
        props.setDisplayLevel(level);
        listStyle->setLevel(props);
    }
    //1.6: KWTextParag::loadOasis
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    if ( !styleName.isEmpty() ) {
        const KoXmlElement* paragraphStyle = context.stylesReader().findStyle( styleName, "paragraph" );
        //QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString() ) : QString();
        //if ( masterPageName.isEmpty() ) masterPageName = "Standard"; // Seems to be a builtin name for the default layout...
        //#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) <<"styleName=" << styleName;
        //#endif
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
        //#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500)<<"with listStyle !";
        //#endif
        paragStyle->setListStyle(*listStyle);
        delete listStyle;
    }
    QTextBlock block = cursor.block();
    paragStyle->applyStyle(block);
    */

    // Add a new block which will become the list-item for the header
    cursor.insertBlock();
    QTextBlock block = cursor.block();

    // Set the paragraph-style on the block
    KoParagraphStyle *userStyle = d->paragraphStyle(styleName);
    if( userStyle ) {
        context.styleStack().setTypeProperties( "paragraph" );
        userStyle->applyStyle(block);
    }

    //1.6: KoTextParag::loadOasisSpan
    bool stripLeadingSpace = true;
    loadSpan(context, parent, cursor, &stripLeadingSpace);

    // Add a new empty block which finish's our list-item block
    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);

    // Add the block as list-item to the list
    list->add(block);

    // Set the list-properties
    if( ! listStyle->hasPropertiesForLevel(level) ) {
        KoListLevelProperties props;
        //props.setStyle(KoListStyle::DecimalItem);
        //props.setListItemSuffix(".");
        props.setStyle( KoListStyle::NoItem );
        //props.setLevel(level);
        //props.setDisplayLevel(level);
        listStyle->setLevel( d->outlineLevel(level, props) );
    }

    // apply the list-style on the block for the defined level
    if (parent.attributeNS ( KoXmlNS::text, "is-list-header", "false") != "true") {
        listStyle->applyStyle(block, level);
    }

    // Remove the first char. This seems to be needed else it crashes for whatever reason :-/
    int endPosition = cursor.position();
    cursor.setPosition(list->item(0).position());
    cursor.deleteChar();
    cursor.setPosition(endPosition - 1);
}
#endif

void KoTextLoader::loadHeading( const KoXmlElement& element, QTextCursor& cursor )
{
#if 0 //1.6:
    int level = tag.attributeNS( KoXmlNS::text, "outline-level", QString() ).toInt();
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
        restartNumbering = tag.attributeNS( KoXmlNS::text, "start-value", QString() ).toInt();
    KoTextParag *parag = createParag( this, lastParagraph, nextParagraph );
    parag->loadOasis( tag, context, styleColl, pos );
    if ( !lastParagraph ) setFirstParag( parag ); // First parag
    lastParagraph = parag;
    if ( listOK ) {
        parag->applyListStyle( context, restartNumbering, true /*ordered*/, true /*heading*/, level );
        context.listStyleStack().pop();
    }
#endif
    int level = element.attributeNS( KoXmlNS::text, "outline-level", "1" ).toInt();
    QString styleName = element.attributeNS( KoXmlNS::text, "style-name", QString() );

    // Get the KoListStyle the name may reference to
    KoListStyle* listStyle = d->textSharedData->listStyle( styleName );

    //context.setCurrentListStyleName(styleName);
    //int level = context.currentListLevel();

    kDebug(32500) << "localName =" << element.localName() << "style-name =" << styleName << "outline-level =" << level;

    // Each header is within a list. That allows us to have them numbered on demand.
    QTextListFormat listformat;
    QTextList* list = cursor.createList( listformat );

    // Add a new block which will become the list-item for the header
    cursor.insertBlock();
    QTextBlock block = cursor.block();

    // Set the paragraph-style on the block
    KoParagraphStyle * paragraphStyle = d->textSharedData->paragraphStyle( styleName, d->stylesDotXml );
    if( paragraphStyle ) {
        paragraphStyle->applyStyle( block );
    }

    //1.6: KoTextParag::loadOasisSpan
    bool stripLeadingSpace = true;
    loadSpan( element, cursor, &stripLeadingSpace );

    // Add a new empty block which finish's our list-item block
    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock( emptyTbf, emptyCf );

#if 0 // TODO tz: I don't understand how should this be used
    // Add the block as list-item to the list
    list->add( block );

    // Set the list-properties
    if( ! listStyle->hasPropertiesForLevel( level ) ) {
        KoListLevelProperties props;
        //props.setStyle(KoListStyle::DecimalItem);
        //props.setListItemSuffix(".");
        props.setStyle( KoListStyle::NoItem );
        //props.setLevel(level);
        //props.setDisplayLevel(level);
        listStyle->setLevel( d->textSharedData->outlineLevel( level, props ) );
    }

    // apply the list-style on the block for the defined level
    if ( element.attributeNS ( KoXmlNS::text, "is-list-header", "false") != "true" ) {
        listStyle->applyStyle( block, level );
    }

    // TODO tz: this needs to be investigated
    // Remove the first char. This seems to be needed else it crashes for whatever reason :-/
    int endPosition = cursor.position();
    cursor.setPosition( list->item(0).position() );
    cursor.deleteChar();
    cursor.setPosition( endPosition - 1 );
#endif
}

#if 0
//1.6: KoTextDocument::loadList
void KoTextLoader::loadList(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    // The optional text:style-name attribute specifies the name of the list style that is applied to the list.
    QString styleName;
    if ( parent.hasAttributeNS( KoXmlNS::text, "style-name" ) ) {
        styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
        d->currentListStyleName = styleName;
    }
    else {
        // If this attribute is not included and therefore no list style is specified, one of the following actions is taken:
        // * If the list is contained within another list, the list style defaults to the style of the surrounding list.
        // * If there is no list style specified for the surrounding list, but the list contains paragraphs that have paragraph styles attached specifying a list style, this list style is used for any of these paragraphs.
        // * An application specific default list style is applied to any other paragraphs.
        styleName = d->currentListStyleName;
    }

    // Get the KoListStyle the name may reference to
    KoListStyle* listStyle = d->listStyle(styleName);
    if( ! listStyle ) { // no such list means we define a default one
        listStyle = new KoListStyle();
        listStyle->setName(styleName);
        d->addStyle(listStyle);
        d->currentListStyleName = styleName;
    }

    // The level specifies the level of the list style.
    int level = d->currentListLevel;

    // Set the style and create the textlist
    QTextListFormat listformat;
    QTextList* list = cursor.createList(listformat);

    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"styleName="<<styleName<<" listStyle="<<(listStyle ? listStyle->name() :"NULL")
            <<" level="<<level<<" hasPropertiesForLevel="<<listStyle->hasPropertiesForLevel(level)
            //<<" style="<<props.style()<<" prefix="<<props.listItemPrefix()<<" suffix="<<props.listItemSuffix()
            <<endl;
    #endif

    // we need at least one item, so add a dummy-item we remove later again
    cursor.insertBlock();
    QTextBlock prev = cursor.block();

    // increment list level by one for nested lists.
    d->currentListLevel = level + 1;

    // Iterate over list items and add them to the textlist
    KoXmlElement e;
    forEachElement(e, parent) {
        if( e.isNull() ) continue;
        if( ( e.tagName() != "list-item" ) || ( e.namespaceURI() != KoXmlNS::text ) ) continue;
        /*
        //TODO handle also the other item properties
        if( e.hasAttributeNS( KoXmlNS::text, "start-value" ) ) {
            int startValue = e.attributeNS(KoXmlNS::text, "start-value", QString()).toInt();
            KoListLevelProperties p = KoListLevelProperties::fromTextList(list);
            p.setStartValue(startValue);
            QTextListFormat f = list->format();
            p.applyStyle(f);
            list->setFormat(f);
        }
        */

        //listStyle->applyStyle(cursor.block(), level + 1);
        //listStyle->applyStyle(cursor.block());
        QTextBlock current = cursor.block();
        list->add(current);

        loadBody(context, e, cursor);

        if( ! listStyle->hasPropertiesForLevel(level) ) { // set default style
            KoListLevelProperties props;
            props.setStyle(KoListStyle::DecimalItem);
            props.setListItemSuffix(".");
            props.setLevel(0);
            listStyle->setLevel(props);
        }
        if( prev != current )
            listStyle->applyStyle(current, level);
    }

    // set the list level back to the previous value
    d->currentListLevel = level;
    /*
    // add the new blocks to the list
    QTextBlock current = cursor.block();
    for(QTextBlock b = prev; b.isValid() && b != current; b = b.next()) {
        list->add(b);
        listStyle->applyStyle(b, level);
    }
    */
    int endPosition = cursor.position();
    cursor.setPosition(list->item(0).position());
    cursor.deleteChar();
    cursor.setPosition(endPosition - 1);
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
    cursor.setBlockFormat(emptyTbf);
    cursor.setCharFormat(emptyCf);
    //cursor.insertBlock(emptyTbf, emptyCf);
}
#endif

void KoTextLoader::loadList( const KoXmlElement& element, QTextCursor& cursor )
{
    // The optional text:style-name attribute specifies the name of the list style that is applied to the list.
    QString styleName;
    if ( element.hasAttributeNS( KoXmlNS::text, "style-name" ) ) {
        styleName = element.attributeNS( KoXmlNS::text, "style-name", QString() );
        d->currentListStyleName = styleName;
    }
    else {
        // If this attribute is not included and therefore no list style is specified, one of the following actions is taken:
        // * If the list is contained within another list, the list style defaults to the style of the surrounding list.
        // * If there is no list style specified for the surrounding list, but the list contains paragraphs that have paragraph styles attached specifying a list style, this list style is used for any of these paragraphs.
        // * An application specific default list style is applied to any other paragraphs.
        styleName = d->currentListStyleName;
    }

    // Get the KoListStyle the name may reference to
    KoListStyle* listStyle = d->textSharedData->listStyle( styleName );
    if( ! listStyle ) { // no such list means we define a default one
        kWarning(32500) << "liststyle" << styleName << "not found";
        listStyle = new KoListStyle();
        listStyle->setName(styleName);
        // TODO d->addStyle(listStyle);
        d->currentListStyleName = styleName;
    }

    // The level specifies the level of the list style.
    int level = d->currentListLevel;

    // Set the style and create the textlist
    QTextListFormat listformat;
    QTextList* list = cursor.createList( listformat );

    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500) << "styleName =" << styleName << "listStyle =" << ( listStyle ? listStyle->name() :"NULL" )
                      <<"level =" << level << "hasPropertiesForLevel =" << listStyle->hasPropertiesForLevel( level )
                    //<<" style="<<props.style()<<" prefix="<<props.listItemPrefix()<<" suffix="<<props.listItemSuffix()
                      ;
    #endif

    // we need at least one item, so add a dummy-item we remove later again
    cursor.insertBlock();
    QTextBlock prev = cursor.block();

    // increment list level by one for nested lists.
    d->currentListLevel = level + 1;

    // Iterate over list items and add them to the textlist
    KoXmlElement e;
    forEachElement( e, element ) {
        if( e.isNull() ) {
            continue;
        }
        if( ( e.tagName() != "list-item" ) || ( e.namespaceURI() != KoXmlNS::text ) ) {
            continue;
        }
        /*
        //TODO handle also the other item properties
        if( e.hasAttributeNS( KoXmlNS::text, "start-value" ) ) {
            int startValue = e.attributeNS(KoXmlNS::text, "start-value", QString()).toInt();
            KoListLevelProperties p = KoListLevelProperties::fromTextList(list);
            p.setStartValue(startValue);
            QTextListFormat f = list->format();
            p.applyStyle(f);
            list->setFormat(f);
        }
        */

        //listStyle->applyStyle(cursor.block(), level + 1);
        //listStyle->applyStyle(cursor.block());
        QTextBlock current = cursor.block();
        list->add( current );

        loadBody( e, cursor );

        if ( ! listStyle->hasPropertiesForLevel( level ) ) { // set default style
            KoListLevelProperties props;
            props.setStyle( KoListStyle::DecimalItem );
            props.setListItemSuffix( "." );
            props.setLevel( 0 );
            listStyle->setLevel( props );
        }
        if ( prev != current ) {
            listStyle->applyStyle( current, level );
        }
    }

    // set the list level back to the previous value
    d->currentListLevel = level;
    /*
    // add the new blocks to the list
    QTextBlock current = cursor.block();
    for(QTextBlock b = prev; b.isValid() && b != current; b = b.next()) {
        list->add(b);
        listStyle->applyStyle(b, level);
    }
    */
    int endPosition = cursor.position();
    cursor.setPosition( list->item( 0 ).position() );
    cursor.deleteChar();
    cursor.setPosition( endPosition - 1 );
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
    cursor.setBlockFormat(emptyTbf);
    cursor.setCharFormat(emptyCf);
    //cursor.insertBlock(emptyTbf, emptyCf);
}

#if 0
//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadSection(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"KoTextLoader::loadSection";
    #endif

    //TODO
    //kdDebug(32500) << "Section found!" << endl;
    //context.fillStyleStack( tag, KoXmlNS::text, "style-name", "section" );
    //lastParagraph = loadOasisText( tag, context, lastParagraph, styleColl, nextParagraph );
    Q_UNUSED(context);
    Q_UNUSED(parent);
    Q_UNUSED(cursor);
}
#endif

void KoTextLoader::loadSection( const KoXmlElement& sectionElem, QTextCursor& cursor )
{
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
//1.6: KoTextParag::loadOasisSpan
void KoTextLoader::loadSpan(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor, bool* stripLeadingSpace)
{
    Q_ASSERT( stripLeadingSpace );
    for (KoXmlNode node = parent.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        KoXmlElement ts = node.toElement();
        const QString localName( ts.localName() );
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;

        // allow loadSpanTag to modify the stylestack
        context.styleStack().save();

        if ( node.isText() )
        {
            QString text = node.toText().data();
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <text> localName=" << localName <<" parent.localName="<<parent.localName()<<" text=" << text;
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
                kDebug(32500) <<"  <span> localName=" << localName;
            #endif
            context.fillStyleStack( ts, KoXmlNS::text, "style-name", "text" );
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            context.styleStack().setTypeProperties( "text" );
            const QString textStyleName = ts.attributeNS( KoXmlNS::text, "style-name", QString() );
            const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.stylesReader().findStyle( textStyleName, "text" );
            KoCharacterStyle *charstyle = 0;
            if( textStyleElem ) {
                #ifdef KOOPENDOCUMENTLOADER_DEBUG
                    kDebug(32500)<<"textStyleName="<<textStyleName;
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
        else if ( isTextNS && localName == "a" ) // text:a
        {
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            QTextCharFormat linkCf(cf); // and copy it to alter it
            linkCf.setAnchor(true);
            linkCf.setAnchorHref(ts.attributeNS(KoXmlNS::xlink, "href"));
            QBrush foreground = linkCf.foreground();
            foreground.setColor(Qt::blue);
            foreground.setStyle(Qt::Dense1Pattern);
            linkCf.setForeground(foreground);
            linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
            linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
            linkCf.setFontItalic(true);
            cursor.setCharFormat(linkCf);
            loadSpan(context, ts, cursor, stripLeadingSpace); // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
        }
        else if ( isTextNS && localName == "line-break" ) // text:line-break
        {
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <line-break> Node localName=" << localName;
            #endif
            //QTextBlockFormat emptyTbf;
            //QTextCharFormat emptyCf;
            //cursor.insertBlock(emptyTbf, emptyCf);
            cursor.insertText( "\n" );
        }
        else if ( isTextNS && localName == "number" ) // text:number
        {
            // This is the number in front of a numbered paragraph,
            // written out to help export filters. We can ignore it.
        }
        else if ( isDrawNS && localName == "frame" ) // draw:frame
        {
            loadFrame(context, ts, cursor);
        }
        else if ( isTextNS && (localName == "date" || localName == "time") )
        {
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("date")) {
                            KoInlineObjectFactory *dateFactory = KoInlineObjectRegistry::instance()->value("date");
                            if (dateFactory) {
                                QString dataStyle = ts.attributeNS(KoXmlNS::style, "data-style-name");
                                QString dateFormat = "";
                                if (!dataStyle.isEmpty()) {
                                    if (context.stylesReader().dataFormats().contains(dataStyle)) {
                                        KoOdfNumberStyles::NumericStyleFormat dataFormat = context.stylesReader().dataFormats().value(dataStyle);
                                        dateFormat = dataFormat.prefix + dataFormat.formatStr + dataFormat.suffix;
                                    }
                                }
                                KoProperties dateProperties;
                                dateProperties.setProperty("fixed", QVariant(ts.attributeNS(KoXmlNS::text, "fixed") == "true"));
                                dateProperties.setProperty("time", ts.attributeNS(KoXmlNS::text, localName + "-value"));
                                dateProperties.setProperty("definition", dateFormat);
                                dateProperties.setProperty("adjust", ts.attributeNS(KoXmlNS::text, localName + "-adjust"));
                                if (dateFormat.isEmpty())
                                    dateProperties.setProperty("displayType", localName);
                                else
                                    dateProperties.setProperty("displayType", "custom");

                                KoInlineObject *dateObject = dateFactory->createInlineObject(&dateProperties);
                                textObjectManager->insertInlineObject(cursor, dateObject);
                            }
                        }
                    }
                }
            }
        }
        else if ( isTextNS && (localName == "page-count" || localName == "page-number") ) {
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("page")) {
                            KoInlineObjectFactory *pageFactory = KoInlineObjectRegistry::instance()->value("page");
                            if (pageFactory) {
                                KoProperties props;
                                if (localName == "page-count")
                                    props.setProperty("count", true);
                                else
                                    props.setProperty("count", false);
                                KoInlineObject *pageObject = pageFactory->createInlineObject(&props);
                                textObjectManager->insertInlineObject(cursor, pageObject);
                            }
                        }
                    }
                }
            }

        }
        else if ( isTextNS && ((localName == "title") || (localName == "subject") || (localName == "keywords")) ) {
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("info")) {
                            KoInlineObjectFactory *infoFactory = KoInlineObjectRegistry::instance()->value("info");
                            if (infoFactory) {
                                KoProperties props;
                                if (localName == "title")
                                    props.setProperty("property", KoInlineObject::Title);
                                else if (localName == "subject")
                                    props.setProperty("property", KoInlineObject::Subject);
                                else if (localName == "keywords")
                                    props.setProperty("property", KoInlineObject::Keywords);
                                KoInlineObject *infoObject = infoFactory->createInlineObject(&props);
                                textObjectManager->insertInlineObject(cursor, infoObject);
                            }
                        }
                    }
                }
            }
        }
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
                    kWarning(32500) << "Ignoring tag " << ts.tagName();
                    context.styleStack().restore();
                    continue;
                }
            }
#else
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  Node localName=" << localName <<" is UNHANDLED";
            #endif
#endif
        }

        // restore the propably by loadSpanTag modified stylestack
        context.styleStack().restore();
    }
}
#endif

void KoTextLoader::loadSpan( const KoXmlElement& element, QTextCursor& cursor, bool* stripLeadingSpace )
{
    Q_ASSERT( stripLeadingSpace );
    for ( KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        KoXmlElement ts = node.toElement();
        const QString localName( ts.localName() );
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;

        if ( node.isText() ) {
            QString text = node.toText().data();
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <text> localName=" << localName <<" parent.localName="<<element.localName()<<" text=" << text;
            #endif
            text = normalizeWhitespace( text.replace('\n', QChar(0x2028)), *stripLeadingSpace );

            if ( text.isEmpty() )
                *stripLeadingSpace = false;
            else
                *stripLeadingSpace = text[text.length() - 1].isSpace();

            cursor.insertText( text );
        }
        else if ( isTextNS && localName == "span" ) // text:span
        {
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <span> localName=" << localName;
            #endif
            QString styleName = ts.attributeNS( KoXmlNS::text, "style-name", QString() );

            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            KoCharacterStyle * characterStyle = d->textSharedData->characterStyle( styleName, d->stylesDotXml );
            if ( characterStyle ) {
                characterStyle->applyStyle( &cursor );
            }
            else {
                kWarning(32500) << "character style " << styleName << " not found";
            }

            loadSpan( ts, cursor, stripLeadingSpace ); // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
        }
        else if ( isTextNS && localName == "s" ) // text:s
        {
            int howmany = 1;
            if ( ts.hasAttributeNS( KoXmlNS::text, "c" ) ) {
                howmany = ts.attributeNS( KoXmlNS::text, "c", QString() ).toInt();
            }
            cursor.insertText( QString().fill( 32, howmany ) );
        }
        else if ( isTextNS && localName == "tab" ) // text:tab
        {
            cursor.insertText( "\t" );
        }
        else if ( isTextNS && localName == "a" ) // text:a
        {
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            QTextCharFormat linkCf( cf ); // and copy it to alter it
            linkCf.setAnchor( true );
            linkCf.setAnchorHref( ts.attributeNS( KoXmlNS::xlink, "href" ) );
            // TODO make configurable ?
            QBrush foreground = linkCf.foreground();
            foreground.setColor( Qt::blue );
            foreground.setStyle( Qt::Dense1Pattern );
            linkCf.setForeground( foreground );
            linkCf.setProperty( KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine );
            linkCf.setProperty( KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine );
            linkCf.setFontItalic( true );
            cursor.setCharFormat( linkCf );
            loadSpan( ts, cursor, stripLeadingSpace ); // recurse
            cursor.setCharFormat( cf ); // restore the cursor char format
        }
        else if ( isTextNS && localName == "line-break" ) // text:line-break
        {
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <line-break> Node localName=" << localName;
            #endif
            //QTextBlockFormat emptyTbf;
            //QTextCharFormat emptyCf;
            //cursor.insertBlock(emptyTbf, emptyCf);
            cursor.insertText( "\n" );
        }
        else if ( isTextNS && localName == "number" ) // text:number
        {
            // TODO is this true?
            // This is the number in front of a numbered paragraph,
            // written out to help export filters. We can ignore it.
        }
        else if ( isDrawNS && localName == "frame" ) // draw:frame
        {
            loadFrame( ts, cursor );
        }
        else if ( isTextNS && ( localName == "date" || localName == "time" ) )
        {
#if 0 // commented out for now
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>( cursor.block().document()->documentLayout() );
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("date")) {
                            KoInlineObjectFactory *dateFactory = KoInlineObjectRegistry::instance()->value("date");
                            if (dateFactory) {
                                QString dataStyle = ts.attributeNS(KoXmlNS::style, "data-style-name");
                                QString dateFormat = "";
                                if (!dataStyle.isEmpty()) {
                                    if (context.stylesReader().dataFormats().contains(dataStyle)) {
                                        KoOdfNumberStyles::NumericStyleFormat dataFormat = context.stylesReader().dataFormats().value(dataStyle);
                                        dateFormat = dataFormat.prefix + dataFormat.formatStr + dataFormat.suffix;
                                    }
                                }
                                KoProperties dateProperties;
                                dateProperties.setProperty("fixed", QVariant(ts.attributeNS(KoXmlNS::text, "fixed") == "true"));
                                dateProperties.setProperty("time", ts.attributeNS(KoXmlNS::text, localName + "-value"));
                                dateProperties.setProperty("definition", dateFormat);
                                dateProperties.setProperty("adjust", ts.attributeNS(KoXmlNS::text, localName + "-adjust"));
                                if (dateFormat.isEmpty())
                                    dateProperties.setProperty("displayType", localName);
                                else
                                    dateProperties.setProperty("displayType", "custom");

                                KoInlineObject *dateObject = dateFactory->createInlineObject(&dateProperties);
                                textObjectManager->insertInlineObject(cursor, dateObject);
                            }
                        }
                    }
                }
            }
#endif
        }
        else if ( isTextNS && (localName == "page-count" || localName == "page-number") ) 
        {
#if 0 // commented out for now
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("page")) {
                            KoInlineObjectFactory *pageFactory = KoInlineObjectRegistry::instance()->value("page");
                            if (pageFactory) {
                                KoProperties props;
                                if (localName == "page-count")
                                    props.setProperty("count", true);
                                else
                                    props.setProperty("count", false);
                                KoInlineObject *pageObject = pageFactory->createInlineObject(&props);
                                textObjectManager->insertInlineObject(cursor, pageObject);
                            }
                        }
                    }
                }
            }
#endif
        }
        else if ( isTextNS && ((localName == "title") || (localName == "subject") || (localName == "keywords")) )
        {
#if 0 // commented out for now
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("info")) {
                            KoInlineObjectFactory *infoFactory = KoInlineObjectRegistry::instance()->value("info");
                            if (infoFactory) {
                                KoProperties props;
                                if (localName == "title")
                                    props.setProperty("property", KoInlineObject::Title);
                                else if (localName == "subject")
                                    props.setProperty("property", KoInlineObject::Subject);
                                else if (localName == "keywords")
                                    props.setProperty("property", KoInlineObject::Keywords);
                                KoInlineObject *infoObject = infoFactory->createInlineObject(&props);
                                textObjectManager->insertInlineObject(cursor, infoObject);
                            }
                        }
                    }
                }
            }
#endif
        }
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
                    kWarning(32500) << "Ignoring tag " << ts.tagName();
                    context.styleStack().restore();
                    continue;
                }
            }
#else
            kDebug(32500) <<"Node '" << localName << "' unhandled";
#endif
        }
    }
}

#if 0
void KoTextLoader::loadFrame(KoTextLoadingContext& context, const KoXmlElement& frameElem, QTextCursor& cursor)
{
    if( ! d->frameLoader )
        d->frameLoader = new KoTextFrameLoader(this);
    d->frameLoader->loadFrame(context, frameElem, cursor);
}
#endif

void KoTextLoader::loadFrame( const KoXmlElement& frameElem, QTextCursor& cursor )
{
}

void KoTextLoader::startBody(int total)
{
    d->bodyProgressTotal += total;
}

void KoTextLoader::processBody()
{
    d->bodyProgressValue++;
    if( d->dt.elapsed() >= d->lastElapsed + 1000 ) { // update only once per second
        d->lastElapsed = d->dt.elapsed();
        Q_ASSERT( d->bodyProgressTotal > 0 );
        const int percent = d->bodyProgressValue * 100 / d->bodyProgressTotal;
        emit sigProgress( percent );
    }
}

void KoTextLoader::endBody()
{
}

#include "KoTextLoader.moc"
