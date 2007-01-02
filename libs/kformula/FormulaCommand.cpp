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

#include "FormulaCommand.h"
#include "FormulaCursor.h"
#include <klocale.h> 

namespace KFormula {

FormulaCommandAdd::FormulaCommandAdd( FormulaCursor* cursor, QList<BasicElement*> added )
                 : QUndoCommand()
{
    m_ownerElement = cursor()->ownerElement();
    m_positionInElement = cursor()->position();
    m_addedElements = added;
    setText( i18n( "Add Elements" ) );
}

void FormulaCommandAdd::redo()
{
    FormulaCursor* cursor = new FormulaCursor( m_ownerElement );
    cursor->setPosition( m_positionInElement );
    
    foreach( BasicElement* tmp, m_addedElements )
        m_ownerElement->insertChild( cursor, tmp );
	
    delete cursor;
}

void FormulaCommandAdd::undo()
{
    foreach( BasicElement* tmp, m_addedElements )
        m_ownerElement->removeElement( tmp );
}



FormulaCommandRemove::FormulaCommandRemove( FormulaCursor* cursor,
                                            QList<BasicElement*> removed )
                    : QUndoCommand()
{
    m_ownerElement = cursor()->ownerElement();
    m_positionInElement = cursor()->position();
    m_removedElements = removed;
    setText( i18n( "Remove Elements" ) );
}

void FormulaCommandRemove::redo()
{
    foreach( BasicElement* tmp, m_removedElements )
        m_ownerElement->removeElement( tmp );
}

void FormulaCommandRemove::undo()
{
    FormulaCursor* cursor = new FormulaCursor( m_ownerElement );
    cursor->setPosition( m_positionInElement );
    
    foreach( BasicElement* tmp, m_removedElements )
        m_ownerElement->insertChild( cursor, tmp );
	
    delete cursor;
}



FormulaCommandReplace::FormulaCommandReplace( FormulaCursor* cursor,
                       QList<BasicElement*> replaced,QList<BasicElement*> replacing )
                     : QUndoCommand()
{
    m_ownerElement = cursor()->ownerElement();
    m_positionInElement = cursor()->position();
    m_replacedElements = replaced;
    m_replacingElements = replacing;
    setText( i18n( "Replace Element" ) );
}

void FormulaCommandReplace::redo()
{
    foreach( BasicElement* tmp, m_replacedElements )
        m_ownerElement->removeElement( tmp );

    FormulaCursor* cursor = new FormulaCursor( m_ownerElement );
    cursor->setPosition( m_positionInElement );
    
    foreach( BasicElement* tmp, m_replacingElements )
        m_ownerElement->insertChild( cursor, tmp );
	
    delete cursor;
}

void FormulaCommandReplace::undo()
{
    foreach( BasicElement* tmp, m_replacingElements )
        m_ownerElement->removeElement( tmp );

    FormulaCursor* cursor = new FormulaCursor( m_ownerElement );
    cursor->setPosition( m_positionInElement );
    
    foreach( BasicElement* tmp, m_replacedElements )
        m_ownerElement->insertChild( cursor, tmp );
	
    delete cursor;
}



FormulaCommandAttribute::FormulaCommandAttribute( FormulaCursor* cursor,
                                                  QHash<QString,QString> attributes )
                       : QUndoCommand()
{
    m_ownerElement = cursor->ownerElement();
    m_attributes = attributes;
    m_oldAttributes = m_ownerElement->attributes();
    QHashIterator<QString, QString> i( m_oldAttributes );
    while( i.hasNext() )
    {
        i.next();
	if( !m_attributes.contains( i.key() ) )
            m_attributes.insert( i.key(), i.value() );
    }

    setText( i18n( "Attribute Changed" ) );
}

void FormulaCommandAttribute::redo()
{
    m_ownerElement->setAttributes( m_attributes );
}

void FormulaCommandAttribute::undo()
{
    m_ownerElement->setAttributes( m_oldAttributes );
}

} //namespace KFormula
