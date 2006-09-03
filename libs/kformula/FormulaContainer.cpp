/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
		      
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

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QClipboard>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QTextStream>
#include <QList>
#include <QKeyEvent>

#include <kdebug.h>
#include <klocale.h>

#include "KoGlobal.h"
#include "BracketElement.h"
#include "contextstyle.h"
#include "FormulaCursor.h"
#include "FormulaElement.h"
#include "FractionElement.h"
#include "kformulacommand.h"
#include "kformulacompatibility.h"
#include "FormulaContainer.h"
#include "MatrixElement.h"
#include "RootElement.h"
#include "SequenceElement.h"
#include "symboltable.h"
#include "SpaceElement.h"
#include "TextElement.h"

#include <assert.h>

KFORMULA_NAMESPACE_BEGIN
using namespace std;


struct Container::Container_Impl {

    Container_Impl( /*Document* doc*/ )
            : dirty( true ), cursorMoved( false )//, document( doc )
    {
    }

    ~Container_Impl()
    {
        delete internCursor;
//        delete m_formulaElement;
//        document = 0;
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
//    FormulaElement* rootElement;

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
//    Document* document;
};

//Document* Container::document() const { return impl->document; }

Container::Container( /*Document* doc,*/ int pos, bool registerMe )
{
//    impl = new Container_Impl( doc );
    m_formulaElement = 0;
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
    m_formulaElement = new FormulaElement( ); 
    impl->activeCursor = impl->internCursor = createCursor();
    recalcLayout();
}


FormulaCursor* Container::createCursor()
{
    return new FormulaCursor( m_formulaElement );
}

/*
KCommandHistory* Container::getHistory() const
{
    return document()->getHistory();
}
*/

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
//    document()->registerFormula( this, pos );
}

void Container::unregisterFormula()
{
//    document()->unregisterFormula( this );
}


void Container::baseSizeChanged( int size, bool owned )
{
/*    if ( owned ) {
        emit baseSizeChanged( size );
    }
    else {
        const ContextStyle& context = document()->getContextStyle();
        emit baseSizeChanged( context.baseSize() );
    }*/
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
//    document()->activate(this);
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
        recalcLayout();
    }
}

void Container::recalcLayout()
{
//    m_formulaElement->layout();
/*    impl->dirty = false;
    ContextStyle& context = impl->document->getContextStyle();
    m_formulaElement->calcSizes( context );

    emit formulaChanged( context.layoutUnitToPixelX( m_formulaElement->getWidth() ),
                         context.layoutUnitToPixelY( m_formulaElement->getHeight() ) );
    emit formulaChanged( context.layoutUnitPtToPt( context.pixelXToPt( m_formulaElement->getWidth() ) ),
                         context.layoutUnitPtToPt( context.pixelYToPt( m_formulaElement->getHeight() ) ) );
    emit cursorMoved( activeCursor() );*/
}

bool Container::isEmpty()
{
    return m_formulaElement->childElements().isEmpty();
}


const SymbolTable& Container::getSymbolTable() const
{
//    return document()->getSymbolTable();
}


void Container::draw( QPainter& painter, const QRectF& r, const QPalette& palette, bool edit )
{
    painter.fillRect( r, palette.base() );
    draw( painter, r, edit );
}


void Container::draw( QPainter& painter, const QRectF& r, bool edit )
{
//    ContextStyle& context = document()->getContextStyle( edit );
//    m_formulaElement->draw( painter, context.pixelToLayoutUnit( r ), context );
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
//    execute( activeCursor()->getElement()->input( this, event ) );
    checkCursor();
}


void Container::performRequest( Request* request )
{
    if ( !hasValidCursor() )
        return;
//    execute( activeCursor()->getElement()->buildCommand( this, request ) );
    checkCursor();
}


void Container::paste()
{
/*    if (!hasValidCursor())
        return;
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* source = clipboard->mimeData();
    if (source->hasFormat( MimeSource::selectionMimeType() ))
    {
        QByteArray data = source->data( MimeSource::selectionMimeType() );
        QDomDocument formula;
        formula.setContent(data);
        paste( formula, i18n("Paste") );
    }*/
}

void Container::paste( const QDomDocument& document, QString desc )
{
    FormulaCursor* cursor = activeCursor();
    QList<BasicElement*> list;
//    list.setAutoDelete( true );
    if ( cursor->buildElementsFromDom( document.documentElement(), list ) ) {
        uint count = list.count();
        // You must not execute an add command that adds nothing.
        if (count > 0) {
            KFCReplace* command = new KFCReplace( desc, this );
            for (uint i = 0; i < count; i++) {
                command->addElement(list.takeAt(0));
            }
            //execute(command);
        }
    }
}

void Container::copy()
{
/*    // read-only cursors are fine for copying.
    FormulaCursor* cursor = activeCursor();
    if (cursor != 0)
    {
        QDomDocument formula;// = document()->createDomDocument();
        cursor->copy( formula );
        QClipboard* clipboard = QApplication::clipboard();
	QMimeData* data = new QMimeData();
	data->setData(MimeSource::selectionMimeType(), formula.toByteArray() );
        clipboard->setMimeData( data );
    }*/
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

const QRectF& Container::boundingRect() const
{
/*    const ContextStyle& context = document()->getContextStyle();
    return QRectF( context.layoutUnitToPixelX( m_formulaElement->getX() ),
                   context.layoutUnitToPixelY( m_formulaElement->getY() ),
                   context.layoutUnitToPixelX( m_formulaElement->getWidth() ),
                   context.layoutUnitToPixelY( m_formulaElement->getHeight() ) );*/
}

const QRectF& Container::coveredRect() const
{/*
    if ( impl->activeCursor != 0 ) {
        const ContextStyle& context = document()->getContextStyle();
        const LuPixelRect& cursorRect = impl->activeCursor->getCursorSize();
        return QRectF( context.layoutUnitToPixelX( m_formulaElement->getX() ),
                      context.layoutUnitToPixelY( m_formulaElement->getY() ),
                      context.layoutUnitToPixelX( m_formulaElement->getWidth() ),
                      context.layoutUnitToPixelY( m_formulaElement->getHeight() ) ) |
            QRectF( context.layoutUnitToPixelX( cursorRect.x() ),
                   context.layoutUnitToPixelY( cursorRect.y() ),
                   context.layoutUnitToPixelX( cursorRect.width() ),
                   context.layoutUnitToPixelY( cursorRect.height() ) );
    }
    return boundingRect();*/
}
/*
double Container::width() const
{
    const ContextStyle& context = document()->getContextStyle();
    return context.layoutUnitPtToPt( context.pixelXToPt( m_formulaElement->getWidth() ) );
}

double Container::height() const
{
    const ContextStyle& context = document()->getContextStyle();
    return context.layoutUnitPtToPt( context.pixelYToPt( m_formulaElement->getHeight() ) );
}*/

double Container::baseline() const
{
/*    const ContextStyle& context = document()->getContextStyle();
    //return context.layoutUnitToPixelY( rootElement()->getBaseline() );
    return context.layoutUnitPtToPt( context.pixelYToPt( m_formulaElement->getBaseline() ) );*/
}

void Container::moveTo( int x, int y )
{
/*    const ContextStyle& context = document()->getContextStyle();
    m_formulaElement->setX( context.pixelToLayoutUnitX( x ) );
    m_formulaElement->setY( context.pixelToLayoutUnitY( y ) );*/
}

int Container::fontSize() const
{
    if ( m_formulaElement->hasOwnBaseSize() ) {
        return m_formulaElement->getBaseSize();
    }
    else {
//        const ContextStyle& context = document()->getContextStyle();
//        return qRound( context.baseSize() );
    }
}

void Container::setFontSize( int pointSize, bool /*forPrint*/ )
{
    if ( m_formulaElement->getBaseSize() != pointSize ) {
        ///execute( new KFCChangeBaseSize( i18n( "Base Size Change" ), this, m_formulaElement, pointSize ) );
    }
}

void Container::setFontSizeDirect( int pointSize )
{
    m_formulaElement->setBaseSize( pointSize );
    recalcLayout();
}

void Container::updateMatrixActions()
{
/*    BasicElement *currentElement = activeCursor()->getElement();
    if ( ( currentElement = currentElement->getParent() ) != 0 )
        document()->wrapper()->enableMatrixActions( dynamic_cast<MatrixElement*>(currentElement) );
    else
        document()->wrapper()->enableMatrixActions( false );*/
}

void Container::save( QDomElement &root )
{
    QDomDocument ownerDoc = root.ownerDocument();
    root.appendChild(m_formulaElement->getElementDom(ownerDoc));
}


/**
 * Loads a formula from the document.
 */
bool Container::load( const QDomElement &fe )
{
    if( fe.isNull() ) {
        kWarning( DEBUGID ) << "Empty element." << endl;
        return false;
    }
    
    FormulaElement* root = new FormulaElement( );
    if (root->buildFromDom(fe)) {
        delete m_formulaElement;
        m_formulaElement = root;
        emit formulaLoaded(m_formulaElement);

        recalcLayout();
        return true;
    }
    else {
        delete root;
        kWarning( DEBUGID ) << "Error constructing element tree." << endl;
    }
}


void Container::saveMathML( QTextStream& stream, bool oasisFormat )
{
/*    if ( !oasisFormat )
    {
        // ### TODO: Are we really using MathML 2.0 or would be MathMl 1.01 enough (like for OO)?
        QDomDocumentType dt = QDomImplementation().createDocumentType( "math",
                                                                       "-//W3C//DTD MathML 2.0//EN",
                                                                       "http://www.w3.org/TR/MathML2/dtd/mathml2.dtd");
        QDomDocument doc( dt );
        doc.insertBefore( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ), doc.documentElement() );
        m_formulaElement->writeMathML( doc, doc, oasisFormat );
        stream << doc;
    }
    else
    {
        QDomDocument doc;
        m_formulaElement->writeMathML( doc, doc, oasisFormat );
        stream << doc;
    }*/
}

bool Container::loadMathML( const QDomDocument &doc, bool oasisFormat )
{
    return loadMathML( doc.documentElement(), oasisFormat );
}

bool Container::loadMathML( const QDomElement &element, bool oasisFormat )
{
/*    const ContextStyle& context = document()->getContextStyle();
    MathML2KFormula filter( element, context, oasisFormat );
    filter.startConversion();
    if (filter.m_error) {
        return false;
    }

    if ( load( filter.getKFormulaDom().documentElement() ) ) {
        getHistory()->clear();
        return true;
    }
    return false;*/
}

/*
QImage Container::drawImage( int width, int height )
{
    ContextStyle& context = document()->getContextStyle( false );
    QRectF rect( m_formulaElement->getX(), m_formulaElement->getY(),
                m_formulaElement->getWidth(), m_formulaElement->getHeight());

    int realWidth = context.layoutUnitToPixelX( m_formulaElement->getWidth() );
    int realHeight = context.layoutUnitToPixelY( m_formulaElement->getHeight() );

    double f = qMax( static_cast<double>( width )/static_cast<double>( realWidth ),
                     static_cast<double>( height )/static_cast<double>( realHeight ) );

    int oldZoom = context.zoomInPercent();
    context.setZoomAndResolution( qRound( oldZoom*f ), KoGlobal::dpiX(), KoGlobal::dpiY() );

    QPixmap pm( context.layoutUnitToPixelX( m_formulaElement->getWidth() ),
                context.layoutUnitToPixelY( m_formulaElement->getHeight() ) );
    pm.fill();
    QPainter paint(&pm);
    m_formulaElement->draw(paint, rect.toRect(), context);
    paint.end();
    // FIXME store and reset zoomedResolution as well as zoom. This is lossy
    context.setZoomAndResolution( oldZoom, KoGlobal::dpiX(), KoGlobal::dpiY() );
    return pm.toImage();
}*/

} // namespace KFormula

using namespace KFormula;
#include "FormulaContainer.moc"
