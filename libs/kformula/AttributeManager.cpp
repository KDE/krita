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

#include "AttributeManager.h"
#include "BasicElement.h"

namespace KFormula {

AttributeManager::AttributeManager()
{
}

AttributeManager::~AttributeManager()
{
}

QVariant AttributeManager::valueOfAttribute( const QString& attribute )
{
    QString value;
    foreach( BasicElement* tmp, m_attributeStack )
    {
        value = tmp->inheritAttribute( attribute );
        if( !value.isEmpty() )
            break;
    }
    
    if( value.isEmpty() )                     // there was no attribute found
        return defaultValueOf( attribute );   // return default value

    
}

void AttributeManager::inheritAttributes( BasicElement* e )
{
    m_attributeStack.prepend( e );

    if( alteringScriptLevel( e ) )
        m_scriptLevel++;
}

void AttributeManager::disinheritAttributes()
{
    if( alteringScriptLevel( m_attributeStack.takeFirst() ) )
        m_scriptLevel--;
}

bool AttributeManager::alteringScriptLevel( const BasicElement* e ) const
{
    // TODO mroot increments scriptLevel for its index by 2 and sets displaystyle to false
    // TODO mtable sets displaystyle to false
	
    switch( e->elementType() )     // some elements alter the script level
    {
        case MultiScript:          // msub, msup, msubsup and mmultiscripts
        case UnderOver:            // munder, mover and munderover
            return true;
            break;
        case Fraction:             // mfrac
	    if( m_displayStyle )
                return true;
        default:
            return false;
    }
}

QVariant AttributeManager::defaultValueOf( const QString& attribute )
{
}

} // namespace KFormula
