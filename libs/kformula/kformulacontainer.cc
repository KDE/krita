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

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QTextStream>
//Added by qt3to4:
#include <Q3PtrList>
#include <QKeyEvent>

#include <kdebug.h>
#include <klocale.h>
#include <kprinter.h>

#include "KoGlobal.h"
#include "bracketelement.h"
#include "contextstyle.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "fractionelement.h"
#include "indexelement.h"
#include "kformulacommand.h"
#include "kformulacompatibility.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "kformulamathmlread.h"
#include "kformulamimesource.h"
#include "matrixelement.h"
#include "rootelement.h"
#include "sequenceelement.h"
#include "symbolelement.h"
#include "symboltable.h"
#include "spaceelement.h"
#include "textelement.h"

#include <assert.h>

KFORMULA_NAMESPACE_BEGIN
using namespace std;


struct Container::Container_Impl {

    Container_Impl( Document* doc )
            : dirty( true ), cursorMoved( false ), document( doc )
    {
    }

    ~Container_Impl()
    {
        delete internCursor;
        delete rootElement;
        document = 0;
    }

    /**
     * If true we need to recalc the formula.
     */
    bool dirty;

    /**
     * Tells whether a request caused the cursor to move.
     */
    bool cursorMoved;

    /**
     * The element tree's root.
     */
    FormulaElement* rootElement;

    /**
     * The active cursor is the one that triggered the last command.
     */
    FormulaCursor* activeCursor;

    /**
     * The cursor that is used if there is no view.
     */
    FormulaCursor* internCursor;

    /**
     * The document we belong to.
     */
    Document* document;
};


FormulaElement* Container::rootElement() const { return impl->rootElement; }
Document* Container::document() const { return impl->document; }

Container::Container( Document* doc, int pos, bool registerMe )
{
    impl = new Container_Impl( doc );
    impl->rootElement = 0;
    if ( registerMe ) {
        registerFormula( pos );
    }
}

Container::~Container()
{
    unregisterFormula();
    delete impl;
    impl = 0;
}


void Container::initialize()
{
    assert( impl->rootElement == 0 );
    impl->rootElement = createMainSequence();
    impl->activeCursor = impl->internCursor = createCursor();
    recalc();
}


FormulaElement* Container::createMainSequence()
{
    return new FormulaElement( this );
}


FormulaCursor* Container::createCursor()
{
    return new FormulaCursor(rootElement());
}


KoCommandHistory* Container::getHistory() const
{
    return document()->getHistory();
}


/**
 * Gets called just before the child is removed from
 * the element tree.
 */
void Container::elementRemoval(BasicElement* child)
{
    emit elementWillVanish(child);
}

/**
 * Gets called whenever something changes and we need to
 * recalc.
 */
void Container::changed()
{
    impl->dirty = true;
}

void Container::cursorHasMoved( FormulaCursor* )
{
    impl->cursorMoved = true;
}

void Container::moveOutLeft( FormulaCursor* cursor )
{
    emit leaveFormula( this, cursor, EXIT_LEFT );
}

void Container::moveOutRight( FormulaCursor* cursor )
{
    emit leaveFormula( this, cursor, EXIT_RIGHT );
}

void Container::moveOutAbove( FormulaCursor* cursor )
{
    emit leaveFormula( this, cursor, EXIT_ABOVE );
}

void Container::moveOutBelow( FormulaCursor* cursor )
{
    emit leaveFormula( this, cursor, EXIT_BELOW );
}

void Container::tell( const QString& msg )
{
    emit statusMsg( msg );
}

void Container::removeFormula( FormulaCursor* cursor )
{
    emit leaveFormula( this, cursor, REMOVE_FORMULA );
}


void Container::registerFormula( int pos )
{
    document()->registerFormula( this, pos );
}

void Container::unregisterFormula()
{
    document()->unregisterFormula( this );
}


void Container::baseSizeChanged( int size, bool owned )
{
    if ( owned ) {
        emit baseSizeChanged( size );
    }
    else {
        const ContextStyle& context = document()->getContextStyle();
        emit baseSizeChanged( context.baseSize() );
    }
}

FormulaCursor* Container::activeCursor()
{
    return impl->activeCursor;
}

const FormulaCursor* Container::activeCursor() const
{
    return impl->activeCursor;
}


/**
 * Tells the formula that a view got the focus and might want to
 * edit the formula.
 */
void Container::setActiveCursor(FormulaCursor* cursor)
{
    document()->activate(this);
    if (cursor != 0) {
        impl->activeCursor = cursor;
    }
    else {
        *(impl->internCursor) = *(impl->activeCursor);
        impl->activeCursor = impl->internCursor;
    }
}


bool Container::hasValidCursor() const
{
    return (impl->activeCursor != 0) && !impl->activeCursor->isReadOnly();
}

void Container::testDirty()
{
    if (impl->dirty) {
        recalc();
    }
}

void Container::recalc()
{
    impl->dirty = false;
    ContextStyle& context = impl->document->getContextStyle();
    rootElement()->calcSizes( context );

    emit formulaChanged( context.layoutUnitToPixelX( rootElement()->getWidth() ),
                         context.layoutUnitToPixelY( rootElement()->getHeight() ) );
    emit formulaChanged( context.layoutUnitPtToPt( context.pixelXToPt( rootElement()->getWidth() ) ),
                         context.layoutUnitPtToPt( context.pixelYToPt( rootElement()->getHeight() ) ) );
    emit cursorMoved( activeCursor() );
}

bool Container::isEmpty()
{
    return rootElement()->countChildren() == 0;
}


const SymbolTable& Container::getSymbolTable() const
{
    return document()->getSymbolTable();
}


void Container::draw( QPainter& painter, const QRect& r, const QPalette& palette, bool edit )
{
    painter.fillRect( r, palette.base() );
    draw( painter, r, edit );
}


void Container::draw( QPainter& painter, const QRect& r, bool edit )
{
    //ContextStyle& context = document()->getContextStyle( painter.device()->devType() == QInternal::Printer );
    ContextStyle& context = document()->getContextStyle( edit );
    rootElement()->draw( painter, context.pixelToLayoutUnit( r ), context );
}


void Container::checkCursor()
{
    if ( impl->cursorMoved ) {
        impl->cursorMoved = false;
        emit cursorMoved( activeCursor() );
    }
}

void Container::input( QKeyEvent* event )
{
    //if ( !hasValidCursor() )
    if ( impl->activeCursor == 0 ) {
        return;
    }
    execute( activeCursor()->getElement()->input( this, event ) );
    checkCursor();
}


void Container::performRequest( Request* request )
{
    if ( !hasValidCursor() )
        return;
    execute( activeCursor()->getElement()->buildCommand( this, request ) );
    checkCursor();
}


void Container::paste()
{
    if (!hasValidCursor())
        return;
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* source = clipboard->mimeData();
    if (source->hasFormat( MimeSource::selectionMimeType() ))
    {
        QByteArray data = source->data( MimeSource::selectionMimeType() );
        QDomDocument formula;
        formula.setContent(data);
        paste( formula, i18n("Paste") );
    }
}

void Container::paste( const QDomDocument& document, QString desc )
{
    FormulaCursor* cursor = activeCursor();
    Q3PtrList<BasicElement> list;
    list.setAutoDelete( true );
    if ( cursor->buildElementsFromDom( document.documentElement(), list ) ) {
        uint count = list.count();
        // You must not execute an add command that adds nothing.
        if (count > 0) {
            KFCReplace* command = new KFCReplace( desc, this );
            for (uint i = 0; i < count; i++) {
                command->addElement(list.take(0));
            }
            execute(command);
        }
    }
}

void Container::copy()
{
    // read-only cursors are fine for copying.
    FormulaCursor* cursor = activeCursor();
    if (cursor != 0)
    {
        QDomDocument formula = document()->createDomDocument();
        cursor->copy( formula );
        QClipboard* clipboard = QApplication::clipboard();
	QMimeData* data = new QMimeData();
	data->setData(MimeSource::selectionMimeType(), formula.toByteArray() );
        clipboard->setMimeData( data );
    }
}

void Container::cut()
{
    if (!hasValidCursor())
        return;
    FormulaCursor* cursor = activeCursor();
    if (cursor->isSelection()) {
        copy();
        DirectedRemove r( req_remove, beforeCursor );
        performRequest( &r );
    }
}


void Container::emitErrorMsg( const QString& msg )
{
    emit errorMsg( msg );
}

void Container::execute(KCommand* command)
{
    if ( command != 0 ) {
        getHistory()->addCommand(command);
    }
}


QRect Container::boundingRect() const
{
    const ContextStyle& context = document()->getContextStyle();
    return QRect( context.layoutUnitToPixelX( rootElement()->getX() ),
                  context.layoutUnitToPixelY( rootElement()->getY() ),
                  context.layoutUnitToPixelX( rootElement()->getWidth() ),
                  context.layoutUnitToPixelY( rootElement()->getHeight() ) );
}

QRect Container::coveredRect()
{
    if ( impl->activeCursor != 0 ) {
        const ContextStyle& context = document()->getContextStyle();
        const LuPixelRect& cursorRect = impl->activeCursor->getCursorSize();
        return QRect( context.layoutUnitToPixelX( rootElement()->getX() ),
                      context.layoutUnitToPixelY( rootElement()->getY() ),
                      context.layoutUnitToPixelX( rootElement()->getWidth() ),
                      context.layoutUnitToPixelY( rootElement()->getHeight() ) ) |
            QRect( context.layoutUnitToPixelX( cursorRect.x() ),
                   context.layoutUnitToPixelY( cursorRect.y() ),
                   context.layoutUnitToPixelX( cursorRect.width() ),
                   context.layoutUnitToPixelY( cursorRect.height() ) );
    }
    return boundingRect();
}

double Container::width() const
{
    const ContextStyle& context = document()->getContextStyle();
    return context.layoutUnitPtToPt( context.pixelXToPt( rootElement()->getWidth() ) );
}

double Container::height() const
{
    const ContextStyle& context = document()->getContextStyle();
    return context.layoutUnitPtToPt( context.pixelYToPt( rootElement()->getHeight() ) );
}

double Container::baseline() const
{
    const ContextStyle& context = document()->getContextStyle();
    //return context.layoutUnitToPixelY( rootElement()->getBaseline() );
    return context.layoutUnitPtToPt( context.pixelYToPt( rootElement()->getBaseline() ) );
}

void Container::moveTo( int x, int y )
{
    const ContextStyle& context = document()->getContextStyle();
    rootElement()->setX( context.pixelToLayoutUnitX( x ) );
    rootElement()->setY( context.pixelToLayoutUnitY( y ) );
}

int Container::fontSize() const
{
    if ( rootElement()->hasOwnBaseSize() ) {
        return rootElement()->getBaseSize();
    }
    else {
        const ContextStyle& context = document()->getContextStyle();
        return qRound( context.baseSize() );
    }
}

void Container::setFontSize( int pointSize, bool /*forPrint*/ )
{
    if ( rootElement()->getBaseSize() != pointSize ) {
        execute( new KFCChangeBaseSize( i18n( "Base Size Change" ), this, rootElement(), pointSize ) );
    }
}

void Container::setFontSizeDirect( int pointSize )
{
    rootElement()->setBaseSize( pointSize );
    recalc();
}

void Container::updateMatrixActions()
{
    BasicElement *currentElement = activeCursor()->getElement();
    if ( ( currentElement = currentElement->getParent() ) != 0 )
        document()->wrapper()->enableMatrixActions( dynamic_cast<MatrixElement*>(currentElement) );
    else
        document()->wrapper()->enableMatrixActions( false );
}

void Container::save( QDomElement &root )
{
    QDomDocument ownerDoc = root.ownerDocument();
    root.appendChild(rootElement()->getElementDom(ownerDoc));
}


/**
 * Loads a formula from the document.
 */
bool Container::load( const QDomElement &fe )
{
    if (!fe.isNull()) {
        FormulaElement* root = createMainSequence();
        if (root->buildFromDom(fe)) {
            delete impl->rootElement;
            impl->rootElement = root;
            emit formulaLoaded(rootElement());

            recalc();
            return true;
        }
        else {
            delete root;
            kWarning( DEBUGID ) << "Error constructing element tree." << endl;
        }
    }
    else {
        kWarning( DEBUGID ) << "Empty element." << endl;
    }
    return false;
}


void Container::saveMathML( QTextStream& stream, bool oasisFormat )
{
    if ( !oasisFormat )
    {
        // ### TODO: Are we really using MathML 2.0 or would be MathMl 1.01 enough (like for OO)?
        QDomDocumentType dt = QDomImplementation().createDocumentType( "math",
                                                                       "-//W3C//DTD MathML 2.0//EN",
                                                                       "http://www.w3.org/TR/MathML2/dtd/mathml2.dtd");
        QDomDocument doc( dt );
        doc.insertBefore( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ), doc.documentElement() );
        rootElement()->writeMathML( doc, doc, oasisFormat );
        stream << doc;
    }
    else
    {
        QDomDocument doc;
        rootElement()->writeMathML( doc, doc, oasisFormat );
        stream << doc;
    }
}

bool Container::loadMathML( const QDomDocument &doc, bool oasisFormat )
{
    const ContextStyle& context = document()->getContextStyle();
    MathML2KFormula filter( doc, context, oasisFormat );
    filter.startConversion();
    if (filter.m_error) {
        return false;
    }

    if ( load( filter.getKFormulaDom().documentElement() ) ) {
        getHistory()->clear();
        return true;
    }
    return false;
}


void Container::print(KPrinter& printer)
{
    //printer.setFullPage(true);
    QPainter painter;
    if (painter.begin(&printer)) {
        rootElement()->draw( painter, LuPixelRect( rootElement()->getX(),
                                                   rootElement()->getY(),
                                                   rootElement()->getWidth(),
                                                   rootElement()->getHeight() ),
                             document()->getContextStyle( false ) );
    }
}

QImage Container::drawImage( int width, int height )
{
    ContextStyle& context = document()->getContextStyle( false );
    QRect rect(impl->rootElement->getX(), impl->rootElement->getY(),
               impl->rootElement->getWidth(), impl->rootElement->getHeight());

    int realWidth = context.layoutUnitToPixelX( impl->rootElement->getWidth() );
    int realHeight = context.layoutUnitToPixelY( impl->rootElement->getHeight() );

    double f = qMax( static_cast<double>( width )/static_cast<double>( realWidth ),
                     static_cast<double>( height )/static_cast<double>( realHeight ) );

    int oldZoom = context.zoom();
    context.setZoomAndResolution( qRound( oldZoom*f ), KoGlobal::dpiX(), KoGlobal::dpiY() );

    kDebug( DEBUGID ) << "Container::drawImage "
                       << "(" << width << " " << height << ")"
                       << "(" << context.layoutUnitToPixelX( impl->rootElement->getWidth() )
                       << " " << context.layoutUnitToPixelY( impl->rootElement->getHeight() ) << ")"
                       << endl;

    QPixmap pm( context.layoutUnitToPixelX( impl->rootElement->getWidth() ),
                context.layoutUnitToPixelY( impl->rootElement->getHeight() ) );
    pm.fill();
    QPainter paint(&pm);
    impl->rootElement->draw(paint, rect, context);
    paint.end();
    context.setZoomAndResolution( oldZoom, KoGlobal::dpiX(), KoGlobal::dpiY() );
    //return pm.convertToImage().smoothScale( width, height );
    return pm.toImage();
}

QString Container::texString()
{
    return rootElement()->toLatex();
}

QString Container::formulaString()
{
    return rootElement()->formulaString();
}

KFORMULA_NAMESPACE_END

using namespace KFormula;
#include "kformulacontainer.moc"
