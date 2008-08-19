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

#include "KoTextLoader.h"

// koffice
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoImageData.h>
#include <KoTextAnchor.h>
#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoVariable.h>
#include <KoInlineNote.h>
#include <KoBookmark.h>
#include <KoBookmarkManager.h>
#include <KoVariableManager.h>
#include <KoInlineTextObjectManager.h>
#include <KoVariableRegistry.h>
#include <KoProperties.h>
#include <KoImageCollection.h>
#include <KoTextBlockData.h>

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
        // the KoOdfLoadingContext.
        bool stylesDotXml;

        int bodyProgressTotal;
        int bodyProgressValue;
        int lastElapsed;
        QTime dt;

        KoListStyle *currentListStyle;
        int currentListLevel;
        bool isList;

        KoStyleManager *styleManager;

        explicit Private( KoShapeLoadingContext & context )
        : context( context )
        , textSharedData( 0 )
        // stylesDotXml says from where the office:automatic-styles are to be picked from:
        // the content.xml or the styles.xml (in a multidocument scenario). It does not
        // decide from where the office:styles are to be picked (always picked from styles.xml).
        // For our use here, stylesDotXml is always false (see ODF1.1 spec §2.1).
        , stylesDotXml( context.odfLoadingContext().useStylesAutoStyles() )
        , bodyProgressTotal( 0 )
        , bodyProgressValue( 0 )
        , lastElapsed( 0 )
        , currentListStyle ( 0 )
        , currentListLevel( 1 )
        , isList ( false )
        , styleManager(0)
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
        d->textSharedData->loadOdfStyles( context.odfLoadingContext(), 0 );
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

void KoTextLoader::loadBody( const KoXmlElement& bodyElem, QTextCursor& cursor )
{
    kDebug(32500) << "";

    const QTextDocument *document = cursor.block().document();
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout *>(document->documentLayout());
    if (layout) {
        d->styleManager = layout->styleManager();
    }

    if ((document->isEmpty()) && (d->styleManager)) {
        QTextBlock block = cursor.block();
        d->styleManager->defaultParagraphStyle()->applyStyle(block);
    }

    startBody( KoXml::childNodesCount( bodyElem ) );
    KoXmlElement tag;
    int cursorPosition;
    bool firstTime = true;
    forEachElement(tag, bodyElem) {
        if ( ! tag.isNull() ) {
            const QString localName = tag.localName();
            if (firstTime) {
                cursorPosition = cursor.position();
            } else {
                cursor.insertBlock();
            }
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
                    KoVariable * var = KoVariableRegistry::instance()->createFromOdf( tag, d->context );

                    if ( var ) {
                        KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>( cursor.block().document()->documentLayout() );
                        if ( layout ) {
                            KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                            if ( textObjectManager ) {
                                KoVariableManager *varManager = textObjectManager->variableManager();
                                if ( varManager ) {
                                    textObjectManager->insertInlineObject( cursor, var );
                                }
                            }
                        }
                    }
                    else {
                        kWarning(32500) << "unhandled text:" << localName;
                    }
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
                if ( localName == "table" ) {
                    loadTable( tag, cursor );
                }
                else {
                    kWarning(32500) << "unhandled table:" << localName;
                }
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
        if ((firstTime) && (cursor.position() != cursorPosition))
            firstTime = false;
        processBody();
    }
    endBody();
}

void KoTextLoader::loadParagraph( const KoXmlElement& element, QTextCursor& cursor )
{
    // TODO use the default style name a default value?
    QString styleName = element.attributeNS( KoXmlNS::text, "style-name", QString() );

    KoParagraphStyle * paragraphStyle = d->textSharedData->paragraphStyle( styleName, d->stylesDotXml );

    if (!paragraphStyle && d->styleManager) {
         // Either the paragraph has no style or the style-name could not be found.
         // Fix up the paragraphStyle to be our default paragraph style in either case.
         if (!styleName.isEmpty())
             kWarning(32500) << "paragraph style " << styleName << "not found - using default style";
         paragraphStyle = d->styleManager->defaultParagraphStyle();
    }

    if ( paragraphStyle ) {
        QTextBlock block = cursor.block();
        // apply the paragraph's list style only if we are in a list and the list context we
        // are in (ie. current list and it's parent lists) does not specify a list style
        paragraphStyle->applyStyle(block, /*applyListStyle*/ d->isList && !d->currentListStyle);
    }
    else {
        kWarning(32500) << "paragraph style " << styleName << " not found";
    }

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    bool stripLeadingSpace = true;
    int pos = cursor.position();
    loadSpan( element, cursor, &stripLeadingSpace );
    if (cursor.position() > pos) { // ok something was loaded
        QTextCursor tempCursor(cursor);
        tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1); // select last char loaded
        if (tempCursor.selectedText() == " " && stripLeadingSpace) {            // if it's a collapsed blankspace
            tempCursor.removeSelectedText();                                    // remove it (ODF1.1 §5.1.1)
        }
    }
    cursor.setCharFormat( cf ); // restore the cursor char format
}

void KoTextLoader::loadHeading( const KoXmlElement& element, QTextCursor& cursor )
{
    int level = qMax(1,element.attributeNS( KoXmlNS::text, "outline-level", "1" ).toInt());
    QString styleName = element.attributeNS( KoXmlNS::text, "style-name", QString() );

    QTextBlock block = cursor.block();

    // Set the paragraph-style on the block
    KoParagraphStyle * paragraphStyle = d->textSharedData->paragraphStyle( styleName, d->stylesDotXml );
    if (!paragraphStyle && d->styleManager) {
         paragraphStyle = d->styleManager->defaultParagraphStyle();
    }
    if ( paragraphStyle ) {
        // apply the paragraph's list style only if we are in a list and the list context we
        // are in (ie. current list and it's parent lists) does not specify a list style
        paragraphStyle->applyStyle(block, /*applyListStyle*/ d->isList && !d->currentListStyle);
    } else {
        kWarning(32500) << "paragraph style " << styleName << " not found";
    }

    KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData());
    if (!blockData)
    {
        // Ok, we must create the KoTextBlockData for this block. Quite logical indeed... Except if KoParagraphStyle does create the block data for us ?
        blockData = new KoTextBlockData();
        block.setUserData(blockData);
    }
    if (blockData)
    {
        blockData->setOutlineLevel(level);
    }

    if (!d->isList) { // apply <text:outline-style> (if present) only if heading is not within a <text:list>
        KoListStyle *outlineStyle = d->styleManager->outlineStyle();
        if (outlineStyle)
            outlineStyle->applyStyle(block, level);
    }

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    bool stripLeadingSpace = true;
    int pos = cursor.position();
    loadSpan( element, cursor, &stripLeadingSpace );
    if (cursor.position() > pos) { // ok something was loaded
        QTextCursor tempCursor(cursor);
        tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1); // select last char loaded
        if (tempCursor.selectedText() == " " && stripLeadingSpace) {            // if it's a collapsed blankspace
            tempCursor.removeSelectedText();                                    // remove it
        }
    }
    cursor.setCharFormat( cf ); // restore the cursor char format
}

void KoTextLoader::loadList( const KoXmlElement& element, QTextCursor& cursor )
{
    // The optional text:style-name attribute specifies the name of the list style that is applied to the list.
    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());

    KoListStyle *prevListStyle = d->currentListStyle; // save a copy
    // if list style is not specified or is invalid, fall back to existing currentListStyle,
    // which is the list style of the containing list
    if (KoListStyle *listStyle = d->textSharedData->listStyle(styleName , d->stylesDotXml))
        d->currentListStyle = listStyle;
    d->isList = true;

    // The level specifies the level of the list style.
    int level = d->currentListLevel;

    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        if (d->currentListStyle)
            kDebug(32500) << "styleName =" << styleName << "listStyle =" << d->currentListStyle->name()
                          <<"level =" << level << "hasLevelProperties =" << d->currentListStyle->hasLevelProperties( level )
                         //<<" style="<<props.style()<<" prefix="<<props.listItemPrefix()<<" suffix="<<props.listItemSuffix()
                          ;
        else
            kDebug(32500) << "styleName =" << styleName << " currentListStyle = 0";
    #endif

    // increment list level by one for nested lists.
    d->currentListLevel = level + 1;

    // Iterate over list items and add them to the textlist
    KoXmlElement e;
    bool firstTime = true;
    forEachElement( e, element ) {
        if( e.isNull() ) {
            continue;
        }
        if( ( e.tagName() != "list-item" ) || ( e.namespaceURI() != KoXmlNS::text ) ) {
            continue;
        }

        if (!firstTime) {
            cursor.insertBlock();
        }

        QTextBlock current = cursor.block();

        loadBody( e, cursor );

        if (!d->currentListStyle) {
            if (cursor.block().textList() == 0 && d->styleManager) {
                // We did not find the list-style and the loadParagraph did not put the block in the list either
                d->styleManager->defaultListStyle()->applyStyle(current, level);
            }
        } else {
            if (cursor.block().textList() == 0) {
                d->currentListStyle->applyStyle(current, level);
            }
            // else: if it's already in a list, it belongs to a sublist. doesn't belong to this level.
        }

        if (firstTime) {
            firstTime = false;
            QTextBlockFormat blockFormat;
            blockFormat.setProperty(KoParagraphStyle::StartNewList, true);
            cursor.mergeBlockFormat(blockFormat);
        }

        if( e.hasAttributeNS( KoXmlNS::text, "start-value" ) ) {
            int startValue = e.attributeNS(KoXmlNS::text, "start-value", QString()).toInt();
            QTextBlockFormat blockFormat;
            blockFormat.setProperty(KoParagraphStyle::ListStartValue, startValue);
            cursor.mergeBlockFormat(blockFormat);
        }
    }

    // set the list level back to the previous value
    d->currentListLevel = level;
    d->currentListStyle = prevListStyle;
    d->isList = false;
}

void KoTextLoader::loadSection( const KoXmlElement& sectionElem, QTextCursor& cursor )
{
}

void KoTextLoader::loadNote( const KoXmlElement& noteElem, QTextCursor& cursor )
{
    kDebug(32500) << "Loading a text:note element.";
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>( cursor.block().document()->documentLayout() );
    if (layout)
    {
        KoInlineNote *note = new KoInlineNote(KoInlineNote::Footnote);
        if (note->loadOdf(noteElem))
        {
            KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
            layout->inlineObjectTextManager()->insertInlineObject(cursor, note);
        }
        else {
            kDebug(32500) << "Error while loading the note !";
        }
    }
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
            text = normalizeWhitespace( text.replace('\n', QChar::LineSeparator), *stripLeadingSpace );

            if ( !text.isEmpty() ) {
                // if present text ends with a space,
                // we can remove the leading space in the next text
                *stripLeadingSpace = text[text.length() - 1].isSpace();
            }

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
        else if ( isTextNS && localName == "note" ) // text:note
        {
            loadNote (ts, cursor);
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

            // TODO make configurable ? Ho, and it will interfere with saving :/
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
        // text:bookmark, text:bookmark-start and text:bookmark-end
        else if ( isTextNS &&  (localName == "bookmark" || localName == "bookmark-start" || localName == "bookmark-end") )
        {
            QString bookmarkName = ts.attribute("name");
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>( cursor.block().document()->documentLayout() );
            if (layout) {
                const QTextDocument *document = cursor.block().document();
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                KoBookmark *bookmark = new KoBookmark(bookmarkName, document);

                if (localName == "bookmark")
                    bookmark->setType(KoBookmark::SinglePosition);
                else if (localName == "bookmark-start")
                    bookmark->setType(KoBookmark::StartBookmark);
                else if (localName == "bookmark-end") {
                    bookmark->setType(KoBookmark::EndBookmark);
                    KoBookmark *startBookmark = layout->inlineObjectTextManager()->bookmarkManager()->retrieveBookmark(bookmarkName);
                    startBookmark->setEndBookmark(bookmark);
                }
                layout->inlineObjectTextManager()->insertInlineObject(cursor, bookmark);
            }
        }
        else if ( isTextNS && localName == "number" ) // text:number
        {
            /*                ODF Spec, §4.1.1, Formatted Heading Numbering
            If a heading has a numbering applied, the text of the formatted number can be included in a
            <text:number> element. This text can be used by applications that do not support numbering of
            headings, but it will be ignored by applications that support numbering.                   */
        }
        else if ( isDrawNS && localName == "frame" ) // draw:frame
        {
            loadFrame( ts, cursor );
        }
        else
        {
            KoVariable * var = KoVariableRegistry::instance()->createFromOdf( ts, d->context );

            if ( var ) {
                KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>( cursor.block().document()->documentLayout() );
                if ( layout ) {
                    KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                    if ( textObjectManager ) {
                        KoVariableManager *varManager = textObjectManager->variableManager();
                        if ( varManager ) {
                            textObjectManager->insertInlineObject( cursor, var );
                        }
                    }
                }
            }
            else {
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
            }
#endif
        }
    }
}

void KoTextLoader::loadTable( const KoXmlElement& tableElem, QTextCursor& cursor )
{
    KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(tableElem, d->context);
    if( !shape ) {
        return;
    }

    KoTextAnchor *anchor = new KoTextAnchor(shape);
    anchor->loadOdfFromShape();
    d->textSharedData->shapeInserted(shape);

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>( cursor.block().document()->documentLayout() );
    if ( layout ) {
        KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
        if ( textObjectManager ) {
            textObjectManager->insertInlineObject( cursor, anchor );
        }
    }
}

void KoTextLoader::loadFrame( const KoXmlElement& frameElem, QTextCursor& cursor )
{
    KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(frameElem, d->context);
    if( !shape ) {
        return;
    }

    KoTextAnchor *anchor = new KoTextAnchor(shape);
    anchor->loadOdfFromShape();
    d->textSharedData->shapeInserted(shape);

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>( cursor.block().document()->documentLayout() );
    if ( layout ) {
        KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
        if ( textObjectManager ) {
            textObjectManager->insertInlineObject( cursor, anchor );
        }
    }
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
