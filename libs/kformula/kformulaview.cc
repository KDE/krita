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

#include <iostream>

#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QWheelEvent>
#include <QKeyEvent>

#include <kapplication.h>
#include <kdebug.h>

#include "basicelement.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "kformulaview.h"

KFORMULA_NAMESPACE_BEGIN

struct View::View_Impl {

    View_Impl(Container* doc, View* view)
            : smallCursor(false), activeCursor(true), cursorHasChanged(true),
              document(doc)
    {
        connect(document, SIGNAL(elementWillVanish(BasicElement*)),
                view, SLOT(slotElementWillVanish(BasicElement*)));
        connect(document, SIGNAL(formulaLoaded(FormulaElement*)),
                view, SLOT(slotFormulaLoaded(FormulaElement*)));
        connect(document, SIGNAL(cursorMoved(FormulaCursor*)),
                view, SLOT(slotCursorMoved(FormulaCursor*)));

        cursor = document->createCursor();
        blinkTimer = new QTimer( view );
        connect( blinkTimer, SIGNAL( timeout() ),
                 view, SLOT( slotBlinkCursor() ) );
        if ( QApplication::cursorFlashTime() > 0 )
            blinkTimer->start( QApplication::cursorFlashTime() / 2 );
    }

    void startTimer()
    {
        if ( QApplication::cursorFlashTime() > 0 )
            blinkTimer->start( QApplication::cursorFlashTime() / 2 );
    }
    
    void stopTimer()
    {
        blinkTimer->stop();
    }
    
    ~View_Impl()
    {
        if ( document->activeCursor() == cursor ) {
            document->setActiveCursor( 0 );
        }
        delete cursor;
        delete blinkTimer;
    }

    /**
     * If set the cursor will never be bigger that the formula.
     */
    bool smallCursor;

    /**
     * Whether the cursor is visible (for blinking)
     */
    bool activeCursor;
    
    /**
     * Whether the cursor changed since the last time
     * we emitted a cursorChanged signal.
     */
    bool cursorHasChanged;

    /**
     * Timer for cursor blinking
     */
    QTimer *blinkTimer;
    
    /**
     * The formula we show.
     */
    Container* document;

    /**
     * Out cursor.
     */
    FormulaCursor* cursor;
};


FormulaCursor* View::cursor() const        { return impl->cursor; }
bool& View::cursorHasChanged()             { return impl->cursorHasChanged; }
bool& View::smallCursor()                  { return impl->smallCursor; }
bool& View::activeCursor()                 { return impl->activeCursor; }
Container* View::container() const { return impl->document; }
void View::startCursorTimer() { impl->startTimer(); }
void View::stopCursorTimer() { impl->stopTimer(); }


View::View(Container* doc)
{
    impl = new View_Impl(doc, this);
    cursor()->calcCursorSize( contextStyle(), smallCursor() );
}

View::~View()
{
    delete impl;
}


QPoint View::getCursorPoint() const
{
    return contextStyle().layoutUnitToPixel( cursor()->getCursorPoint().toPoint() );
}

void View::setReadOnly(bool ro)
{
    cursor()->setReadOnly(ro);
}


void View::calcCursor()
{
    cursor()->calcCursorSize( contextStyle(), smallCursor() );
}


void View::draw(QPainter& painter, const QRect& rect, const QPalette& palette)
{
    container()->draw( painter, rect, palette, true );
    if ( cursorVisible() ) {
        cursor()->draw( painter, contextStyle(), smallCursor(), activeCursor() );
    }
}

void View::draw(QPainter& painter, const QRect& rect)
{
    container()->draw( painter, rect, true );
    if ( cursorVisible() ) {
        cursor()->draw( painter, contextStyle(), smallCursor(), activeCursor() );
    }
}

void View::keyPressEvent( QKeyEvent* event )
{
    container()->input( event );
}


void View::focusInEvent(QFocusEvent*)
{
    //cursor()->calcCursorSize( contextStyle(), smallCursor() );
    container()->setActiveCursor(cursor());
    activeCursor() = true;
    startCursorTimer();
    smallCursor() = false;
    emitCursorChanged();
}

void View::focusOutEvent(QFocusEvent*)
{
    //container()->setActiveCursor(0);
    activeCursor() = false;
    stopCursorTimer();
    smallCursor() = true;
    emitCursorChanged();
}

void View::mousePressEvent( QMouseEvent* event )
{
    const ContextStyle& context = contextStyle();
    mousePressEvent( event, context.pixelToLayoutUnit( event->pos() ) );
}

void View::mouseReleaseEvent( QMouseEvent* event )
{
    const ContextStyle& context = contextStyle();
    mouseReleaseEvent( event, context.pixelToLayoutUnit( event->pos() ) );
}

void View::mouseDoubleClickEvent( QMouseEvent* event )
{
    const ContextStyle& context = contextStyle();
    mouseDoubleClickEvent( event, context.pixelToLayoutUnit( event->pos() ) );
}

void View::mouseMoveEvent( QMouseEvent* event )
{
    const ContextStyle& context = contextStyle();
    mouseMoveEvent( event, context.pixelToLayoutUnit( event->pos() ) );
}

void View::wheelEvent( QWheelEvent* event )
{
    const ContextStyle& context = contextStyle();
    wheelEvent( event, context.pixelToLayoutUnit( event->pos() ) );
}
/*
void View::mousePressEvent( QMouseEvent* event, const QPointF& pos )
{
    const ContextStyle& context = contextStyle();
    mousePressEvent( event, context.ptToLayoutUnitPix( pos ) );
}

void View::mouseReleaseEvent( QMouseEvent* event, const QPointF& pos )
{
    const ContextStyle& context = contextStyle();
    mouseReleaseEvent( event, context.ptToLayoutUnitPix( pos ) );
}

void View::mouseDoubleClickEvent( QMouseEvent* event, const QPointF& pos )
{
    const ContextStyle& context = contextStyle();
    mouseDoubleClickEvent( event, context.ptToLayoutUnitPix( pos ) );
}

void View::mouseMoveEvent( QMouseEvent* event, const QPointF& pos )
{
    const ContextStyle& context = contextStyle();
    mouseMoveEvent( event, context.ptToLayoutUnitPix( pos ) );
}

void View::wheelEvent( QWheelEvent* event, const QPointF& pos )
{
    const ContextStyle& context = contextStyle();
    wheelEvent( event, context.ptToLayoutUnitPix( pos ) );
}

*/
void View::mousePressEvent( QMouseEvent* event, const QPointF& pos )
{
    int flags = movementFlag( event->modifiers() );
    cursor()->mousePress( pos, flags );
    emitCursorChanged();
}

void View::mouseReleaseEvent( QMouseEvent* event, const QPointF& pos )
{
    int flags = movementFlag( event->modifiers() );
    cursor()->mouseRelease( pos, flags );
    emitCursorChanged();
}

void View::mouseDoubleClickEvent( QMouseEvent*, const QPointF& )
{
    cursor()->moveRight( WordMovement );
    cursor()->moveLeft( SelectMovement | WordMovement );
    emitCursorChanged();
}

void View::mouseMoveEvent( QMouseEvent* event, const QPointF& pos )
{
    int flags = movementFlag( event->modifiers() );
    cursor()->mouseMove( pos, flags );
    emitCursorChanged();
}

void View::wheelEvent( QWheelEvent*, const QPointF& )
{
}


void View::slotCursorMoved(FormulaCursor* c)
{
    if (c == cursor()) {
        cursorHasChanged() = true;
        emitCursorChanged();
    }
}

void View::slotFormulaLoaded(FormulaElement* formula)
{
    cursor()->formulaLoaded(formula);
}

void View::slotElementWillVanish(BasicElement* element)
{
    cursor()->elementWillVanish(element);
    emitCursorChanged();
}

void View::slotBlinkCursor()
{
    activeCursor() = ! activeCursor();
    emitCursorChanged();
}

void View::slotSelectAll()
{
    cursor()->moveHome(WordMovement);
    cursor()->moveEnd(SelectMovement | WordMovement);
    emitCursorChanged();
}


void View::moveLeft( int flag )
{
    cursor()->moveLeft( flag );
    emitCursorChanged();
}

void View::moveRight( int flag )
{
    cursor()->moveRight( flag );
    emitCursorChanged();
}

void View::moveUp( int flag )
{
    cursor()->moveUp( flag );
    emitCursorChanged();
}

void View::moveDown( int flag )
{
    cursor()->moveDown( flag );
    emitCursorChanged();
}


void View::moveHome( int flag )
{
    cursor()->moveHome( flag );
    emitCursorChanged();
}

void View::moveEnd( int flag )
{
    cursor()->moveEnd( flag );
    emitCursorChanged();
}


void View::setSmallCursor(bool small)
{
    smallCursor() = small;
}

bool View::isHome() const
{
    return cursor()->isHome();
}

bool View::isEnd() const
{
    return cursor()->isEnd();
}

void View::eraseSelection( Direction direction )
{
    DirectedRemove r( req_remove, direction );
    container()->performRequest( &r );
}

void View::addText( QString str )
{
    TextRequest r( str );
    container()->performRequest( &r );
}

void View::emitCursorChanged()
{
    if (cursor()->hasChanged() || cursorHasChanged()) {
        getDocument()->updateMatrixActions();
        cursor()->clearChangedFlag();
        cursorHasChanged() = false;
        cursor()->calcCursorSize( contextStyle(), smallCursor() );
        activeCursor() = true;
        startCursorTimer();
    }
    emit cursorChanged(cursorVisible(), cursor()->isSelection());
}

const ContextStyle& View::contextStyle() const
{
    return container()->document()->getContextStyle();
}

bool View::cursorVisible()
{
    return !cursor()->isReadOnly() || cursor()->isSelection();
}

KFORMULA_NAMESPACE_END

using namespace KFormula;
#include "kformulaview.moc"
