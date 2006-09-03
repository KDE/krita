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
   Boston, MA 02110-1301, USA.
*/

#ifndef PADDEDELEMENT_H
#define PADDEDELEMENT_H

#include "SequenceElement.h"

namespace KFormula {

class PaddedElement : public SequenceElement {
    typedef SequenceElement inherited;
    enum SizeType { NoSize, RelativeSize, AbsoluteSize, PixelSize, WidthRelativeSize, HeightRelativeSize };
public:
    PaddedElement( BasicElement* parent = 0 );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& style,
						    ContextStyle::TextStyle tstyle,
						    ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

protected:
    virtual bool readAttributesFromMathMLDom(const QDomElement& element);

private:
    virtual QString getElementName() const { return "mpadded"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const ;

    double readSizeAttribute( const QString& str, SizeType* st, bool* relative );
    double getSize( const QString& str, SizeType* st );
    double str2size( const QString& str, SizeType* st, SizeType type );
    void writeSizeAttribute( QDomElement element, const QString& str,
                             SizeType st, bool relative, double s ) const ;
    luPixel calcSize( const ContextStyle& context, SizeType type,
                      bool relative, double length, luPixel width,
                      luPixel height, luPixel defvalue );

    SizeType m_widthType;
    double m_width;
    SizeType m_lspaceType;
    double m_lspace;
    SizeType m_heightType;
    double m_height;
    SizeType m_depthType;
    double m_depth;

    bool m_widthRelative;
    bool m_lspaceRelative;
    bool m_heightRelative;
    bool m_depthRelative;
};

} // namespace KFormula

#endif // PADDEDELEMENT_H
