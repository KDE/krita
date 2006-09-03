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

#ifndef OPERATORELEMENT_H
#define OPERATORELEMENT_H

#include "tokenelement.h"

KFORMULA_NAMESPACE_BEGIN

class OperatorElement : public TokenElement {
    typedef TokenElement inherited;
public:
    OperatorElement( BasicElement* parent = 0 );
    void setForm( FormType type );

private:
    virtual bool readAttributesFromMathMLDom( const QDomElement &element );
    virtual QString getElementName() const { return "mo"; }
    void writeMathMLAttributes( QDomElement& element ) const ;
    void writeSizeAttribute( QDomElement& element, const QString &attr, SizeType type, double length ) const ;

    FormType m_form;
    SizeType m_lspaceType;
    double m_lspace;
    SizeType m_rspaceType;
    double m_rspace;
    SizeType m_maxSizeType;
    double m_maxSize;
    SizeType m_minSizeType;
    double m_minSize;
    bool m_fence;
    bool m_separator;
    bool m_stretchy;
    bool m_symmetric;
    bool m_largeOp;
    bool m_movableLimits;
    bool m_accent;

    bool m_customForm;
    bool m_customFence;
    bool m_customSeparator;
    bool m_customLSpace;
    bool m_customRSpace;
    bool m_customStretchy;
    bool m_customSymmetric;
    bool m_customMaxSize;
    bool m_customMinSize;
    bool m_customLargeOp;
    bool m_customMovableLimits;
    bool m_customAccent;

};

KFORMULA_NAMESPACE_END

#endif // OPERATORELEMENT_H
