/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
 * Boston, MA 02110-1301, USA.
*/

#include <q3ptrlist.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3MemArray>

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <KoDocument.h>

#include "contextstyle.h"
#include "creationstrategy.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "sequenceelement.h"
#include "symboltable.h"
#include "symbolaction.h"

KFORMULA_NAMESPACE_BEGIN


static const int CURRENT_SYNTAX_VERSION = 1;
// Make sure an appropriate DTD is available in www/koffice/DTD if changing this value
static const char * CURRENT_DTD_VERSION = "1.3";

/**
 * The creation strategy that we are going to use.
 */
static OrdinaryCreationStrategy creationStrategy;


int FormulaList::compareItems( Q3PtrCollection::Item a, Q3PtrCollection::Item b )
{
    double ya = static_cast<Container*>( a )->getDocumentY();
    double yb = static_cast<Container*>( b )->getDocumentY();
    if ( fabs( ya-yb ) < 1e-4 ) {
        double xa = static_cast<Container*>( a )->getDocumentX();
        double xb = static_cast<Container*>( b )->getDocumentX();
        if ( xa < xb ) return -1;
        if ( xa > xb ) return 1;
        return 0;
    }
    if ( ya < yb ) return -1;
    return 1;
}


Document::Document( QObject *parent, const char *name,
                    const QStringList &/*args*/ )
    : QObject( parent, name ), m_wrapper( 0 ), m_formula( 0 )
{
    m_contextStyle = new ContextStyle;
    SequenceElement::setCreationStrategy( &creationStrategy );
    formulae.setAutoDelete( false );
}


Document::~Document()
{
    // Destroy remaining formulae. We do it backward because
    // the formulae remove themselves from this document upon
    // destruction.
    int count = formulae.count();
    for ( int i=count-1; i>=0; --i ) {
        delete formulae.at( i );
    }
    delete m_contextStyle;
}


bool Document::hasFormula()
{
    return ( m_formula != 0 ) && ( m_formula->activeCursor() != 0 );
}


Container* Document::createFormula( int pos, bool registerMe )
{
    Container* formula = new Container( this, pos, registerMe );
    formula->initialize();
    return formula;
}


Q3PtrListIterator<Container> Document::formulas()
{
    return Q3PtrListIterator<Container>( formulae );
}


int Document::formulaPos( Container* formula )
{
    return formulae.find( formula );
}


Container* Document::formulaAt( uint pos )
{
    return formulae.at( pos );
}


int Document::formulaCount()
{
    return formulae.count();
}


bool Document::loadXML( const QDomDocument& doc )
{
    //clear();
    QDomElement root = doc.documentElement();

    // backward compatibility
    if ( root.tagName() == "FORMULA" ) {
        Container* formula = newFormula( 0 );
        return formula->load( root );
    }

    QDomNode node = root.firstChild();
    if ( node.isElement() ) {
        QDomElement element = node.toElement();
        if ( element.tagName() == "FORMULASETTINGS" ) {
            if ( !loadDocumentPart( element ) ) {
                return false;
            }
        }
        node = node.nextSibling();
    }
    uint number = 0;
    while ( !node.isNull() ) {
        if ( node.isElement() ) {
            QDomElement element = node.toElement();
            Container* formula = newFormula( number );
            if ( !formula->load( element ) ) {
                return false;
            }
            number += 1;
        }
        node = node.nextSibling();
    }
    return formulae.count() > 0;
}

bool Document::loadOasis( const QDomDocument& doc )
{
   // ### TODO: not finished!
    KFormula::Container* formula = newFormula( 0 );
    return formula->loadMathML( doc, true );
}

bool Document::loadDocumentPart( QDomElement /*node*/ )
{
    return true;
}

QDomDocument Document::saveXML()
{
    QDomDocument doc = createDomDocument();
    QDomElement root = doc.documentElement();
    root.appendChild( saveDocumentPart( doc ) );
    uint count = formulae.count();
    for ( uint i=0; i<count; ++i ) {
        formulae.at( i )->save( root );
    }
    return doc;
}


QDomElement Document::saveDocumentPart( QDomDocument& doc )
{
    QDomElement settings = doc.createElement( "FORMULASETTINGS" );
    return settings;
}


QDomDocument Document::createDomDocument()
{
    return KoDocument::createDomDocument( "kformula", "KFORMULA",
                                          CURRENT_DTD_VERSION );
}

void Document::registerFormula( Container* f, int pos )
{
    if ( ( pos > -1 ) &&
         ( static_cast<uint>( pos ) < formulae.count() ) ) {
        formulae.insert( pos, f );
        //emit sigInsertFormula( f, pos );
    }
    else {
        formulae.append( f );
        //emit sigInsertFormula( f, formulae.count()-1 );
    }
}

void Document::unregisterFormula( Container* f )
{
    if ( m_formula == f ) {
        m_formula = 0;
    }
    formulae.removeRef( f );
}

void Document::activate(Container* f)
{
    m_formula = f;
}


void Document::sortFormulaList()
{
    formulae.sort();
}


Container* Document::newFormula( uint number )
{
    if ( number < formulae.count() ) {
        return formulae.at( number );
    }
    return createFormula();
}


double Document::getXResolution() const
{
    return m_contextStyle->zoomedResolutionX();
}
double Document::getYResolution() const
{
    return m_contextStyle->zoomedResolutionY();
}

const SymbolTable& Document::getSymbolTable() const
{
    return m_contextStyle->symbolTable();
}

ContextStyle& Document::getContextStyle( bool edit )
{
    m_contextStyle->setEdit( edit );
    return *m_contextStyle;
}

void Document::setZoomAndResolution( int zoom, int dpiX, int dpiY )
{
    m_contextStyle->setZoomAndResolution( zoom, dpiX, dpiY );
}

void Document::newZoomAndResolution( bool updateViews, bool /*forPrint*/ )
{
    if ( updateViews ) {
        recalc();
    }
}

void Document::setZoomAndResolution( int zoom,
                                     double zoomX, double zoomY,
                                     bool updateViews, bool forPrint )
{
    if ( getContextStyle( !forPrint ).setZoomAndResolution( zoom, zoomX, zoomY, updateViews, forPrint ) && updateViews ) {
        recalc();
    }
}


SymbolType Document::leftBracketChar()
{
    return m_wrapper->leftBracketChar();
}

SymbolType Document::rightBracketChar()
{
    return m_wrapper->rightBracketChar();
}


void Document::setEnabled( bool enabled )
{
    m_wrapper->setEnabled( enabled );
}


KoCommandHistory* Document::getHistory() const
{
    return m_wrapper->getHistory();
}


void Document::recalc()
{
    for ( Container* f = formulae.first();
          f != 0;
          f=formulae.next() ) {
        f->recalc();
    }
}


void Document::updateConfig()
{
    m_wrapper->updateConfig();
    recalc();
}


void Document::introduceWrapper( DocumentWrapper* wrapper, bool init )
{
    m_wrapper = wrapper;
    m_contextStyle->readConfig( wrapper->config(), init );
    m_contextStyle->init( init );
}


//////////////////////////////////////////////////////////////////////////////

DocumentWrapper::DocumentWrapper( KConfig* config,
                                  KActionCollection* collection,
                                  KoCommandHistory* history )
    : m_document( 0 ),
      m_leftBracketChar( LeftRoundBracket ),
      m_rightBracketChar( RightRoundBracket ),
      m_config( config ),
      m_hasActions( collection != 0 )
{
    if ( m_hasActions ) {
        createActions( collection );
        enableMatrixActions( false );
    }
    setCommandStack( history );
}


DocumentWrapper::~DocumentWrapper()
{
    delete m_document;
    if ( m_ownHistory ) {
        delete m_history;
    }

    if ( m_hasActions )
    {
        m_config->setGroup("General");
        m_config->writeEntry("syntaxHighlighting", m_syntaxHighlightingAction->isChecked() );
    }
}


void DocumentWrapper::document( Document* document, bool init )
{
    m_document = document;
    m_document->introduceWrapper( this, init );
    initSymbolNamesAction();
    m_config->setGroup("General");
    if ( m_hasActions )
    {
        m_syntaxHighlightingAction->setChecked( m_config->readEntry("syntaxHighlighting", true ) );
        if ( !m_syntaxHighlightingAction->isChecked() )
            toggleSyntaxHighlighting();
    }
    else if ( m_config->readEntry("syntaxHighlighting", true ) )
    {
        m_document->m_contextStyle->setSyntaxHighlighting( true );
        // Only to notify all views. We don't expect to get new values.
        m_document->recalc();
    }
}


void DocumentWrapper::setCommandStack( KoCommandHistory* history )
{
    if ( history == 0 ) {
        m_history = new KoCommandHistory;
        m_ownHistory = true;
    }
    else {
        m_history = history;
        m_ownHistory = false;
    }
}


void DocumentWrapper::createActions( KActionCollection* collection )
{
    KGlobal::dirs()->addResourceType( "toolbar",
                                      KStandardDirs::kde_default("data") +
                                      "kformula/pics/" );

    m_addNegThinSpaceAction = new KAction( i18n( "Add Negative Thin Space" ),
                                    0,
                                    this, SLOT( addNegThinSpace() ),
                                    collection, "formula_addnegthinspace") ;
    m_addThinSpaceAction = new KAction( i18n( "Add Thin Space" ),
                                    0,
                                    this, SLOT( addThinSpace() ),
                                    collection, "formula_addthinspace") ;
    m_addMediumSpaceAction = new KAction( i18n( "Add Medium Space" ),
                                    0,
                                    this, SLOT( addMediumSpace() ),
                                    collection, "formula_addmediumspace" );
    m_addThickSpaceAction = new KAction( i18n( "Add Thick Space" ),
                                    0,
                                    this, SLOT( addThickSpace() ),
                                    collection, "formula_addthickspace" );
    m_addQuadSpaceAction = new KAction( i18n( "Add Quad Space" ),
                                    0,
                                    this, SLOT( addQuadSpace() ),
                                    collection, "formula_addquadspace" );

    m_addIntegralAction = new KAction(i18n("Add Integral"),
                                    "int",
                                    0,
                                    this, SLOT(addIntegral()),
                                    collection, "formula_addintegral");
    m_addSumAction      = new KAction(i18n("Add Sum"),
                                    "sum",
                                    0,
                                    this, SLOT(addSum()),
                                    collection, "formula_addsum");
    m_addProductAction  = new KAction(i18n("Add Product"),
                                    "prod",
                                    0,
                                    this, SLOT(addProduct()),
                                    collection, "formula_addproduct");
    m_addRootAction     = new KAction(i18n("Add Root"),
                                    "sqrt",
                                    0,
                                    this, SLOT(addRoot()),
                                    collection, "formula_addroot");
    m_addFractionAction = new KAction(i18n("Add Fraction"),
                                    "frac",
                                    0,
                                    this, SLOT(addFraction()),
                                    collection, "formula_addfrac");
    m_addBracketAction  = new KAction(i18n("Add Bracket"),
                                    "paren",
                                    0,
                                    this, SLOT(addDefaultBracket()),
                                    collection,"formula_addbra");
    m_addSBracketAction = new KAction(i18n("Add Square Bracket"),
                                    "brackets",
                                    0,
                                    this, SLOT(addSquareBracket()),
                                    collection,"formula_addsqrbra");
    m_addCBracketAction = new KAction(i18n("Add Curly Bracket"),
                                    "math_brace",
                                    0,
                                    this, SLOT(addCurlyBracket()),
                                    collection,"formula_addcurbra");
    m_addAbsAction      = new KAction(i18n("Add Abs"),
                                    "abs",
                                    0,
                                    this, SLOT(addLineBracket()),
                                    collection,"formula_addabsbra");

    m_addMatrixAction   = new KAction(i18n("Add Matrix..."),
                                    "matrix",
                                    0,
                                    this, SLOT(addMatrix()),
                                    collection, "formula_addmatrix");

    m_addOneByTwoMatrixAction   = new KAction(i18n("Add 1x2 Matrix"),
                                    "onetwomatrix",
                                    0,
                                    this, SLOT(addOneByTwoMatrix()),
                                    collection, "formula_add_one_by_two_matrix");


    m_addUpperLeftAction  = new KAction(i18n("Add Upper Left Index"),
                                      "lsup",
                                      0,
                                      this, SLOT(addUpperLeftIndex()),
                                      collection, "formula_addupperleft");
    m_addLowerLeftAction  = new KAction(i18n("Add Lower Left Index"),
                                      "lsub",
                                      0,
                                      this, SLOT(addLowerLeftIndex()),
                                      collection, "formula_addlowerleft");
    m_addUpperRightAction = new KAction(i18n("Add Upper Right Index"),
                                      "rsup",
                                      0,
                                      this, SLOT(addUpperRightIndex()),
                                      collection, "formula_addupperright");
    m_addLowerRightAction = new KAction(i18n("Add Lower Right Index"),
                                      "rsub",
                                      0,
                                      this, SLOT(addLowerRightIndex()),
                                      collection, "formula_addlowerright");

    m_addGenericUpperAction = new KAction(i18n("Add Upper Index"),
                                      "gsup",
                                              /*CTRL + Qt::Key_U*/0,
                                      this, SLOT(addGenericUpperIndex()),
                                      collection, "formula_addupperindex");
    m_addGenericLowerAction = new KAction(i18n("Add Lower Index"),
                                      "gsub",
                                      0,
                                      this, SLOT(addGenericLowerIndex()),
                                      collection, "formula_addlowerindex");

    m_addOverlineAction = new KAction(i18n("Add Overline"),
                                          "over",
                                          0,
                                          this, SLOT(addOverline()),
                                          collection, "formula_addoverline");
    m_addUnderlineAction = new KAction(i18n("Add Underline"),
                                           "under",
                                           0,
                                           this, SLOT(addUnderline()),
                                           collection, "formula_addunderline");

    m_addMultilineAction = new KAction(i18n("Add Multiline"),
                                           "multiline",
                                           0,
                                           this, SLOT(addMultiline()),
                                           collection, "formula_addmultiline");

    m_removeEnclosingAction = new KAction(i18n("Remove Enclosing Element"),
                                        0,
                                        this, SLOT(removeEnclosing()),
                                        collection, "formula_removeenclosing");

    m_makeGreekAction = new KAction(i18n("Convert to Greek"),
                                  0,
                                  this, SLOT(makeGreek()),
                                  collection, "formula_makegreek");

    m_appendColumnAction = new KAction( i18n( "Append Column" ),
                                            "inscol",
                                            0,
                                            this, SLOT( appendColumn() ),
                                            collection, "formula_appendcolumn" );
    m_insertColumnAction = new KAction( i18n( "Insert Column" ),
                                            "inscol",
                                            0,
                                            this, SLOT( insertColumn() ),
                                            collection, "formula_insertcolumn" );
    m_removeColumnAction = new KAction( i18n( "Remove Column" ),
                                            "remcol",
                                            0,
                                            this, SLOT( removeColumn() ),
                                            collection, "formula_removecolumn" );
    m_appendRowAction = new KAction( i18n( "Append Row" ),
                                         "insrow",
                                         0,
                                         this, SLOT( appendRow() ),
                                         collection, "formula_appendrow" );
    m_insertRowAction = new KAction( i18n( "Insert Row" ),
                                         "insrow",
                                         0,
                                         this, SLOT( insertRow() ),
                                         collection, "formula_insertrow" );
    m_removeRowAction = new KAction( i18n( "Remove Row" ),
                                         "remrow",
                                         0,
                                         this, SLOT( removeRow() ),
                                         collection, "formula_removerow" );

    m_syntaxHighlightingAction = new KToggleAction(i18n("Syntax Highlighting"),
                                                 0,
                                                 this, SLOT(toggleSyntaxHighlighting()),
                                                 collection, "formula_syntaxhighlighting");
    //m_syntaxHighlightingAction->setChecked( m_contextStyle->syntaxHighlighting() );

    m_formatBoldAction = new KToggleAction( i18n( "&Bold" ), "text_bold",
                                                0, //CTRL + Qt::Key_B,
                                                this, SLOT( textBold() ),
                                                collection, "formula_format_bold" );
    m_formatItalicAction = new KToggleAction( i18n( "&Italic" ), "text_italic",
                                                  0, //CTRL + Qt::Key_I,
                                                  this, SLOT( textItalic() ),
                                                  collection, "formula_format_italic" );
    m_formatBoldAction->setEnabled( false );
    m_formatItalicAction->setEnabled( false );

    QStringList delimiter;
    delimiter.append(QString("("));
    delimiter.append(QString("["));
    delimiter.append(QString("{"));
    delimiter.append(QString("<"));
    delimiter.append(QString("/"));
    delimiter.append(QString("\\"));
    delimiter.append(QString("|"));
    delimiter.append(QString(" "));
    delimiter.append(QString(")"));
    delimiter.append(QString("]"));
    delimiter.append(QString("}"));
    delimiter.append(QString(">"));
    m_leftBracket = new KSelectAction(i18n("Left Delimiter"),
                                    0, this, SLOT(delimiterLeft()),
                                    collection, "formula_typeleft");
    m_leftBracket->setItems(delimiter);
    //leftBracket->setCurrentItem(0);

    delimiter.clear();
    delimiter.append(QString(")"));
    delimiter.append(QString("]"));
    delimiter.append(QString("}"));
    delimiter.append(QString(">"));
    delimiter.append(QString("/"));
    delimiter.append(QString("\\"));
    delimiter.append(QString("|"));
    delimiter.append(QString(" "));
    delimiter.append(QString("("));
    delimiter.append(QString("["));
    delimiter.append(QString("{"));
    delimiter.append(QString("<"));
    m_rightBracket = new KSelectAction(i18n("Right Delimiter"),
                                     0, this, SLOT(delimiterRight()),
                                     collection, "formula_typeright");
    m_rightBracket->setItems(delimiter);
    //rightBracket->setCurrentItem(0);

    m_insertSymbolAction = new KAction(i18n("Insert Symbol"),
                                           "key_enter",
                                           /*CTRL + Qt::Key_I*/0,
                                           this, SLOT(insertSymbol()),
                                           collection, "formula_insertsymbol");
    m_symbolNamesAction = new SymbolAction(i18n("Symbol Names"),
                                               0, this, SLOT(symbolNames()),
                                               collection, "formula_symbolnames");

    QStringList ff;
    ff.append( i18n( "Normal" ) );
    ff.append( i18n( "Script" ) );
    ff.append( i18n( "Fraktur" ) );
    ff.append( i18n( "Double Struck" ) );
    m_fontFamily = new KSelectAction(i18n("Font Family"),
                                         0, this, SLOT(fontFamily()),
                                         collection, "formula_fontfamily");
    m_fontFamily->setItems( ff );
    m_fontFamily->setEnabled( false );
}


void DocumentWrapper::paste()
{
    if (hasFormula()) {
        formula()->paste();
    }
}

void DocumentWrapper::copy()
{
    if (hasFormula()) {
        formula()->copy();
    }
}

void DocumentWrapper::cut()
{
    if (hasFormula()) {
        formula()->cut();
    }
}

void DocumentWrapper::undo()
{
    m_history->undo();
}

void DocumentWrapper::redo()
{
    m_history->redo();
}

void DocumentWrapper::addNegThinSpace()
{
    if (hasFormula()) {
        SpaceRequest r( NEGTHIN );
        formula()->performRequest( &r );
    }
}
void DocumentWrapper::addThinSpace()
{
    if (hasFormula()) {
        SpaceRequest r( THIN );
        formula()->performRequest( &r );
    }
}
void DocumentWrapper::addMediumSpace()
{
    if (hasFormula()) {
        SpaceRequest r( MEDIUM );
        formula()->performRequest( &r );
    }
}
void DocumentWrapper::addThickSpace()
{
    if (hasFormula()) {
        SpaceRequest r( THICK );
        formula()->performRequest( &r );
    }
}
void DocumentWrapper::addQuadSpace()
{
    if (hasFormula()) {
        SpaceRequest r( QUAD );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addDefaultBracket()
{
    if (hasFormula()) {
        BracketRequest r( m_leftBracketChar, m_rightBracketChar );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addBracket( SymbolType left, SymbolType right )
{
    if (hasFormula()) {
        BracketRequest r( left, right );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addParenthesis()
{
    if (hasFormula()) {
        BracketRequest r( LeftRoundBracket, RightRoundBracket );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addSquareBracket()
{
    if (hasFormula()) {
        BracketRequest r( LeftSquareBracket, RightSquareBracket );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addCurlyBracket()
{
    if (hasFormula()) {
        BracketRequest r( LeftCurlyBracket, RightCurlyBracket );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addLineBracket()
{
    if (hasFormula()) {
        BracketRequest r( LeftLineBracket, RightLineBracket );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addFraction()
{
    if (hasFormula()) {
        Request r( req_addFraction );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addRoot()
{
    if (hasFormula()) {
        Request r( req_addRoot );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addIntegral()
{
    if (hasFormula()) {
        SymbolRequest r( Integral );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addProduct()
{
    if (hasFormula()) {
        SymbolRequest r( Product );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addSum()
{
    if (hasFormula()) {
        SymbolRequest r( Sum );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addMatrix( uint rows, uint columns )
{
    if (hasFormula()) {
        MatrixRequest r( rows, columns );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addOneByTwoMatrix()
{
    if (hasFormula()) {
        Request r( req_addOneByTwoMatrix );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addNameSequence()
{
    if (hasFormula()) {
        Request r( req_addNameSequence );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addLowerLeftIndex()
{
    if (hasFormula()) {
        IndexRequest r( lowerLeftPos );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addUpperLeftIndex()
{
    if (hasFormula()) {
        IndexRequest r( upperLeftPos );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addLowerRightIndex()
{
    if (hasFormula()) {
        IndexRequest r( lowerRightPos );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addUpperRightIndex()
{
    if (hasFormula()) {
        IndexRequest r( upperRightPos );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addGenericLowerIndex()
{
    if (hasFormula()) {
        IndexRequest r( lowerMiddlePos );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addGenericUpperIndex()
{
    if (hasFormula()) {
        IndexRequest r( upperMiddlePos );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addOverline()
{
    if (hasFormula()) {
        Request r( req_addOverline );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addUnderline()
{
    if (hasFormula()) {
        Request r( req_addUnderline );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::addMultiline()
{
    if (hasFormula()) {
        Request r( req_addMultiline );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::removeEnclosing()
{
    if (hasFormula()) {
        DirectedRemove r( req_removeEnclosing, beforeCursor );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::makeGreek()
{
    if (hasFormula()) {
        Request r( req_makeGreek );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::insertSymbol()
{
    if ( hasFormula() &&
         m_document->m_contextStyle->symbolTable().contains( m_selectedName ) ) {
        QChar ch = m_document->m_contextStyle->symbolTable().unicode( m_selectedName );
        if ( ch != QChar::Null ) {
            TextCharRequest r( ch, true );
            formula()->performRequest( &r );
        }
        else {
            TextRequest r( m_selectedName );
            formula()->performRequest( &r );
        }
    }
}

void DocumentWrapper::insertSymbol( QString name )
{
    if ( hasFormula() ) {
        if ( m_document->m_contextStyle->symbolTable().contains( name ) ) {
            QChar ch = m_document->m_contextStyle->symbolTable().unicode( name );
            if ( ch != QChar::Null ) {
                TextCharRequest r( ch, true );
                formula()->performRequest( &r );
                return;
            }
        }
        TextRequest r( name );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::appendColumn()
{
    if ( hasFormula() ) {
        Request r( req_appendColumn );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::insertColumn()
{
    if ( hasFormula() ) {
        Request r( req_insertColumn );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::removeColumn()
{
    if ( hasFormula() ) {
        Request r( req_removeColumn );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::appendRow()
{
    if ( hasFormula() ) {
        Request r( req_appendRow );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::insertRow()
{
    if ( hasFormula() ) {
        Request r( req_insertRow );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::removeRow()
{
    if ( hasFormula() ) {
        Request r( req_removeRow );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::toggleSyntaxHighlighting()
{
    m_document->m_contextStyle->setSyntaxHighlighting( m_syntaxHighlightingAction->isChecked() );
    // Only to notify all views. We don't expect to get new values.
    m_document->recalc();
}

void DocumentWrapper::textBold()
{
    if ( hasFormula() ) {
        CharStyleRequest r( req_formatBold,
                            getFormatBoldAction()->isChecked(),
                            getFormatItalicAction()->isChecked() );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::textItalic()
{
    if ( hasFormula() ) {
        CharStyleRequest r( req_formatItalic,
                            getFormatBoldAction()->isChecked(),
                            getFormatItalicAction()->isChecked() );
        formula()->performRequest( &r );
    }
}

void DocumentWrapper::delimiterLeft()
{
    QString left = m_leftBracket->currentText();
    switch ( left.at(0).latin1() ) {
    case '[':
    case ']':
    case '{':
    case '}':
    case '<':
    case '>':
    case '(':
    case ')':
    case '/':
    case '\\':
        m_leftBracketChar = static_cast<SymbolType>( left.at(0).latin1() );
        break;
    case '|':
        m_leftBracketChar = LeftLineBracket;
        break;
    case ' ':
        m_leftBracketChar = EmptyBracket;
        break;
    }
}

void DocumentWrapper::delimiterRight()
{
    QString right = m_rightBracket->currentText();
    switch ( right.at(0).latin1() ) {
    case '[':
    case ']':
    case '{':
    case '}':
    case '<':
    case '>':
    case '(':
    case ')':
    case '/':
    case '\\':
        m_rightBracketChar = static_cast<SymbolType>( right.at(0).latin1() );
        break;
    case '|':
        m_rightBracketChar = RightLineBracket;
        break;
    case ' ':
        m_rightBracketChar = EmptyBracket;
        break;
    }
}

void DocumentWrapper::symbolNames()
{
    m_selectedName = m_symbolNamesAction->currentText();
}


void DocumentWrapper::fontFamily()
{
    if ( hasFormula() ) {
        int i = m_fontFamily->currentItem();
        CharFamily cf = anyFamily;
        switch( i ) {
        case 0: cf = normalFamily; break;
        case 1: cf = scriptFamily; break;
        case 2: cf = frakturFamily; break;
        case 3: cf = doubleStruckFamily; break;
        }
        CharFamilyRequest r( cf );
        formula()->performRequest( &r );
    }
}


void DocumentWrapper::initSymbolNamesAction()
{
    if ( m_hasActions ) {
        const SymbolTable& st = m_document->m_contextStyle->symbolTable();

        QStringList names = st.allNames();
        //QStringList i18nNames;
        Q3ValueList<QFont> fonts;
        Q3MemArray<uchar> chars( names.count() );

        int i = 0;
        for ( QStringList::Iterator it = names.begin();
              it != names.end();
              ++it, ++i ) {
            QChar ch = st.unicode( *it );
            //i18nNames.push_back( i18n( ( *it ).latin1() ) );

            fonts.append( st.font( ch ) );
            chars[ i ] = st.character( ch );
        }
        m_symbolNamesAction->setSymbols( names, fonts, chars );
        m_selectedName = names[0];
    }
}


void DocumentWrapper::setEnabled( bool enabled )
{
    kDebug( DEBUGID ) << "DocumentWrapper::setEnabled " << enabled << endl;
    getAddNegThinSpaceAction()->setEnabled( enabled );
    getMakeGreekAction()->setEnabled( enabled );
    getAddGenericUpperAction()->setEnabled( enabled );
    getAddGenericLowerAction()->setEnabled( enabled );
    getAddOverlineAction()->setEnabled( enabled );
    getAddUnderlineAction()->setEnabled( enabled );
    getRemoveEnclosingAction()->setEnabled( enabled );
    getInsertSymbolAction()->setEnabled( enabled );
    getAddThinSpaceAction()->setEnabled( enabled );
    getAddMediumSpaceAction()->setEnabled( enabled );
    getAddThickSpaceAction()->setEnabled( enabled );
    getAddQuadSpaceAction()->setEnabled( enabled );
    getAddBracketAction()->setEnabled( enabled );
    getAddSBracketAction()->setEnabled( enabled );
    getAddCBracketAction()->setEnabled( enabled );
    getAddAbsAction()->setEnabled(enabled);
    getAddFractionAction()->setEnabled( enabled );
    getAddRootAction()->setEnabled( enabled );
    getAddSumAction()->setEnabled( enabled );
    getAddProductAction()->setEnabled( enabled );
    getAddIntegralAction()->setEnabled( enabled );
    getAddMatrixAction()->setEnabled( enabled );
    getAddOneByTwoMatrixAction()->setEnabled( enabled );
    getAddUpperLeftAction()->setEnabled( enabled );
    getAddLowerLeftAction()->setEnabled( enabled );
    getAddUpperRightAction()->setEnabled( enabled );
    getAddLowerRightAction()->setEnabled( enabled );
    
    getAddGenericUpperAction()->setEnabled( enabled );
    getAddGenericLowerAction()->setEnabled( enabled );
    

    if ( enabled ) {
        getAddGenericUpperAction()->
            setShortcut( KShortcut( Qt::CTRL + Qt::Key_U ) );
        getAddGenericLowerAction()->
            setShortcut( KShortcut( Qt::CTRL + Qt::Key_L ) );
        getRemoveEnclosingAction()->
            setShortcut( KShortcut( Qt::CTRL + Qt::Key_R ) );
        getMakeGreekAction()->
            setShortcut( KShortcut( Qt::CTRL + Qt::Key_G ) );
        getInsertSymbolAction()->
            setShortcut( KShortcut( Qt::CTRL + Qt::Key_I ) );
    }
    else {
        getAddGenericUpperAction()->setShortcut( KShortcut() );
        getAddGenericLowerAction()->setShortcut( KShortcut() );
        getRemoveEnclosingAction()->setShortcut( KShortcut() );
        getMakeGreekAction()->setShortcut( KShortcut() );
        getInsertSymbolAction()->setShortcut( KShortcut() );
    }
}

void DocumentWrapper::enableMatrixActions( bool b)
{
    getAppendColumnAction()->setEnabled( b );
    getInsertColumnAction()->setEnabled( b );
    getRemoveColumnAction()->setEnabled( b );
    getAppendRowAction()->setEnabled( b );
    getInsertRowAction()->setEnabled( b );
    getRemoveRowAction()->setEnabled( b );
}

void DocumentWrapper::updateConfig()
{
    m_syntaxHighlightingAction->
        setChecked( m_document->m_contextStyle->syntaxHighlighting() );
    initSymbolNamesAction();
}


KFORMULA_NAMESPACE_END

using namespace KFormula;
#include "kformuladocument.moc"
