/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef STYLEELEMENT_H
#define STYLEELEMENT_H

#include "kformuladefs.h"
#include "TokenStyleElement.h"

KFORMULA_NAMESPACE_BEGIN

class StyleElement : public TokenStyleElement {
    typedef TokenStyleElement inherited;
public:
	
    StyleElement( BasicElement* parent = 0 );

protected:

//    virtual bool readAttributesFromMathMLDom( const QDomElement &element );

    virtual void setStyleSize( const ContextStyle& context, StyleAttributes& style );
    virtual void setStyleVariant( StyleAttributes& style );
    virtual void setStyleBackground( StyleAttributes& style );
    virtual void resetStyle( StyleAttributes& style );

private:
    virtual QString elementName() const { return "mstyle"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const ;

    void readSizeAttribute( const QString& str, SizeType* st, double* s );
    void writeSizeAttribute( QDomElement element, const QString& str, SizeType st, double s ) const ;

    double sizeFactor( const ContextStyle& context, SizeType st, double length, double defvalue );

    int m_scriptLevel;
    SizeType m_scriptMinSizeType;
    double m_scriptSizeMultiplier;
    double m_scriptMinSize;
    QColor m_background;
    SizeType m_veryVeryThinMathSpaceType;
    double m_veryVeryThinMathSpace;
    SizeType m_veryThinMathSpaceType;
    double m_veryThinMathSpace;
    SizeType m_thinMathSpaceType;
    double m_thinMathSpace;
    SizeType m_mediumMathSpaceType;
    double m_mediumMathSpace;
    SizeType m_thickMathSpaceType;
    double m_thickMathSpace;
    SizeType m_veryThickMathSpaceType;
    double m_veryThickMathSpace;
    SizeType m_veryVeryThickMathSpaceType;
    double m_veryVeryThickMathSpace;
    bool m_displayStyle;
    bool m_customScriptLevel;
    bool m_relativeScriptLevel;
    bool m_customDisplayStyle;
    bool m_customScriptSizeMultiplier;
    bool m_customBackground;
};

KFORMULA_NAMESPACE_END

#endif // TOKENSTYLEELEMENT_H
