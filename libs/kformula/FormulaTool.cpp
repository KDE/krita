/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net> 
  
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

#include "FormulaTool.h"
#include "FormulaShape.h"
#include "FormulaCursor.h"
#include "BasicElement.h"
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <QKeyEvent>

namespace KFormula {

FormulaTool::FormulaTool( KoCanvasBase* canvas ) : KoTool( canvas ),
						   m_formulaShape( 0 ),
						   m_formulaCursor( 0 )
{
}

FormulaTool::~FormulaTool()
{
    if( m_formulaCursor )
        delete m_formulaCursor;
}

void FormulaTool::activate( bool temporary )
{
    Q_UNUSED(temporary);
    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach( KoShape* shape, selection->selectedShapes().toList())
    {
        m_formulaShape = dynamic_cast<FormulaShape*>( shape );
        if( m_formulaShape )
            break;
    }
    if( m_formulaShape == 0 )  // none found
    {
        emit sigDone();
        return;
    }

    m_formulaCursor = new FormulaCursor( m_formulaShape->formulaElement() );
}

void FormulaTool::deactivate()
{
    m_formulaShape = 0;
    delete m_formulaCursor;
    m_formulaCursor = 0;
}

void FormulaTool::paint( QPainter &painter, KoViewConverter &converter) const
{
    // TODO do view conversions with converter
    m_formulaCursor->paint( painter );
}

void FormulaTool::mousePressEvent( KoPointerEvent *event )
{
// TODO implement the action and the elementAt method in FormulaShape
//   m_formulaCursor->setCursorTo( m_formulaShape->elementAt( ) );
//
//
//   from the old FormulaCursor implementation
/*
    FormulaElement* formula = getElement()->formula();
    formula->goToPos( this, pos );

    setCursorToElement( m_container->childElementAt( pos ) );
    if (flag & SelectMovement) {
        setSelection(true);
        if (getMark() == -1) {
            setMark(getPos());
        }
    }
    else {
        setSelection(false);
        setMark(getPos());
    }
*/
}

void FormulaTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    // TODO select whole element 
}

void FormulaTool::mouseMoveEvent( KoPointerEvent *event )
{
    // TODO find the old implementation and use it
    //
    // the old implementation
/*    setSelection(true);
    BasicElement* element = getElement();
    int mark = getMark();

   FormulaElement* formula = getElement()->formula();
    formula->goToPos( this, point );
    BasicElement* newElement = getElement();
    int pos = getPos();

    BasicElement* posChild = 0;
    BasicElement* markChild = 0;
    while (element != newElement) {
        posChild = newElement;
        newElement = newElement->getParent();
        if (newElement == 0) {
            posChild = 0;
            newElement = getElement();
            markChild = element;
            element = element->getParent();
        }
    }

    if (dynamic_cast<SequenceElement*>(element) == 0) {
        element = element->getParent();
        element->selectChild(this, newElement);
    }
    else {
        if (posChild != 0) {
            element->selectChild(this, posChild);
            pos = getPos();
        }
        if (markChild != 0) {
            element->selectChild(this, markChild);
            mark = getMark();
        }
        if (pos == mark) {
            if ((posChild == 0) && (markChild != 0)) {
                mark++;
            }
            else if ((posChild != 0) && (markChild == 0)) {
                mark--;
            }
        }
        else if (pos < mark) {
            if (posChild != 0) {
                pos--;
            }
        }
        setTo(element, pos, mark);
    }*/
}

void FormulaTool::mouseReleaseEvent( KoPointerEvent *event )
{
    // TODO what should happen here ?
}

void FormulaTool::keyPressEvent( QKeyEvent *event )
{
    m_formulaCursor->setWordMovement( event->modifiers() & Qt::ControlModifier );
    m_formulaCursor->setSelecting( event->modifiers() & Qt::ShiftModifier );
    	    
    switch( event->key() )                           // map key to movement or action
    { 
        case Qt::Key_Backspace:
            remove( true );
            break;
        case Qt::Key_Delete:
	    remove( false );
            break;
        case Qt::Key_Left:
	    m_formulaCursor->moveLeft();
            break;
        case Qt::Key_Up:
            m_formulaCursor->moveUp();
            break;
        case Qt::Key_Right:
	    m_formulaCursor->moveRight();
            break;
        case Qt::Key_Down:
	    m_formulaCursor->moveDown();
            break;
        case Qt::Key_End:
	    m_formulaCursor->moveEnd();
            break;
        case Qt::Key_Home:
	    m_formulaCursor->moveHome();
            break;
/*        default:
            if( event->text().length() == 0 )
                return;
            insertText( event->text() );*/
    }
    
    event->accept();
}

void FormulaTool::keyReleaseEvent( QKeyEvent *event )
{
    event->accept();
}

void FormulaTool::remove( bool backSpace )
{
    if( m_formulaCursor->hasSelection() )  // remove the selection
    {
	 // TODO set the cursor according to backSpace
//        m_formulaCursor->setCursorTo( );
    }
    else                                  // remove only the current element
    {
//        m_formulaCursor->currentElement()->parentElement()->removeChild( m_currentElement );
    }
}

} // namespace KFormula

