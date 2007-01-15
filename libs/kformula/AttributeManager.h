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

#include <QStack>
#include <QFont>
#include <QVariant>
class KoViewConverter;
class QPaintDevice;

namespace FormulaShape {

class BasicElement;
	
enum MathVariant {
    Normal,
    Bold,
    Italic,
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

enum NamedSpaces { 
    NegativeVeryVeryThinMathSpace,
    NegativeVeryThinMathSpace,
    NegativeThinMathSpace,
    NegativeMediumMathSpace,
    NegativeThickMathSpace,
    NegativeVeryThickMathSpace,
    NegativeVeryVeryThickMathSpace,
    VeryVeryThinMathSpace,
    VeryThinMathSpace,
    ThinMathSpace,
    MediumMathSpace,
    ThickMathSpace,
    VeryThickMathSpace,
    VeryVeryThickMathSpace
};

/** Enum encoding all possibilities to align */
enum Align {
    Left /**< Align to the left*/,
    Center /**< Align to the center*/,
    Right /**< Align to the right*/
};

/** Enum encoding all states of  mo's form attribute */
enum Form {
    Prefix /**< mo is a prefix*/,
    Infix /**< mo is a infix - used for all cases where it's not prefix or postfix*/,
    Postfix /**< mo is a postfix*/
};

/** Enum encoding all states of mspace's linebreak attribute */
enum LineBreak {
    Auto /**< Renderer should use default linebreaking algorithm*/,
    NewLine /**< Start a new line and do not indent*/,
    IndentingNewLine /**< Start a new line and do indent*/,
    NoBreak /**< Do not allow a linebreak here*/,
    GoodBreak /**< If a linebreak is needed on the line, here is a good spot*/,
    BadBreak /**< If a linebreak is needed on the line, try to avoid breaking here*/
};

/**
 * @short manages all the attributes, used by the elements to obtain attribute values
 *
 * The AttributeManager is the central point dealing with the MathML attributes and
 * their values. It is in fact something like a StyleManager. As the normal elements
 * only have a general hash list of the elements they hold, there is the need for a
 * class that manages conversion and heritage of the attributes during the painting
 * phase. These are the two main tasks of AttributeManager.
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

    /// The destructor
    ~AttributeManager();

    /**
     * Obtain a value for attribute
     * @param attribute A string with the attribute to look up
     * @return QVariant with the value
     */
    QVariant valueOf( const QString& attribute ) const;

    /**
     * Inherit the attributes of an element
     * @param element The BasicElement to herit from
     */
    void inheritAttributes( const BasicElement* element );

    /// Disenherit the attributes currently on top of the stack
    void disinheritAttributes();

    /// Obtain the @r current scriptlevel
    int scriptLevel() const;
 
    /// Obtain the @r current displystyle
    bool displayStyle() const;

    /// Set the KoViewConverter to use
    void setViewConverter( KoViewConverter* converter );

    /// Set the QPaintDevice to use
    void setPaintDevice( QPaintDevice* paintDevice );

protected:
    /**
     * Alter the current scriptlevel on the stack's top
     * @param element The BasicElement that effects the changes to the scriptLevel
     */
    void alterScriptLevel( const BasicElement* element );
    QVariant parseValue( const QString& value ) const;

    /**
     * Fill the heritage stack with all the attributes valid for element
     * @param element The BasicElement to build up the heritage stack for
     */
    void buildHeritageOf( const BasicElement* element );

    /**
     * Calculates the pt values of a passes em or ex value
     * @param value The em or ex value to be used for calculation
     * @param isEm Indicates whether to calculate an ex or em value
     * @return The calculated pt value
     */
    double calculateEmExUnits( double value, bool isEm ) const;

    QVariant parseMetrics( const QString& value ) const;

    int formValue( const QString& value ) const;

    int mathVariantValue( const QString& value ) const;

    int alignValue( const QString& value ) const;

    double mathSpaceValue( const QString& value ) const;

private:
    /// The stack of attribute heritage, the topmost element is the currently active
    QStack<const BasicElement*> m_attributeStack;

    /// The stack for the current scriptlevel
    QStack<int> m_scriptLevelStack;

    /// The current font
    QFont m_currentFont; 

    /// The KoViewConverter used to determine the point values of pixels
    KoViewConverter* m_viewConverter;

    /// The QPaintDevice we are currently painting on - needed for em/ ex units
    QPaintDevice* m_paintDevice;
};

} // namespace FormulaShape

#endif // ATTRIBUTEMANAGER_H
