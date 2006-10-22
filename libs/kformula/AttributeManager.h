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

#ifndef ATTRIBUTEMANAGER_H
#define ATTRIBUTEMANAGER_H

#include <QList>

namespace KFormula {

class BasicElement;
	
enum MathVariant {
    Normal,
    Bold,
    BoldItalic,
    DoubleStruck,
    BoldFraktur,
    Script,
    BoldScript,
    Fraktur,
    SansSerif,
    BoldSansSerif,
    SansSerifItalic,
    SansSerifBoldItalic,
    Monospace    
};

enum Align {
    Left,
    Center,
    Right
};

/**
 * @short A manager the elements can retrieve their attribute information from
 *
 * The AttributeManager is the central point dealing with the MathML attributes and
 * their values. As the normal elements only have a general hash list of the elements
 * they hold, there is the need for a class that manages conversion and heritage of
 * the attributes during the painting phase. These are the two main tasks of 
 * AttributeManager.
 * The AttributeManager is always called when an element needs a value to work with.
 * The AttributeManager looks for that value in its stack according to the heritage
 * and if it does not find an appropriated value it returns a default.
 * The AttributeManager stores its stacked BasicElements in a QList rather than a
 * QStack because the former is faster doing a prepend().
 * For conversion from QString to a QVariant mostly the build in QVariant methods
 * are used. For the rest that can not be converted using QVariant the parseValue()
 * method is used.
 *
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @since 2.0
 */
class AttributeManager {
public:
    /// The constructor
    AttributeManager();

    QVariant valueOfAttribute( const QString& attribute );

    /**
     * Inherit the attributes of an element
     * @param e The BasicElement to herit from
     * @param increaseScriptLevel Indicates whether the scriptLevel should be changed
     */
    void inheritAttributes( BasicElement* e, bool increaseScriptLevel );
    void disinheritAttributes( bool decreaseScriptLevel );

protected:
    QVariant defaultValueOf( const QString& attribute );
    bool alteringScriptLevel( const BasicElement* e ) const;
    QVariant parseValue( const QString& value ) const;

private:
    int m_scriptLevel;
    QList<BasicElement*> m_heritageList;
};

} // namespace KFormula

#endif // ATTRIBUTEMANAGER_H
