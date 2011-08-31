/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2009 Jeremias Epperlein <jeeree@web.de>
 
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

#include "FractionElement.h"
#include "FormulaCursor.h"
#include "AttributeManager.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <QPainter>
#include <kdebug.h>

FractionElement::FractionElement( BasicElement* parent ) : FixedElement( parent )
{
    m_numerator = new RowElement( this );
    m_denominator = new RowElement( this );
    m_lineThickness = 1.0;
}

FractionElement::~FractionElement()
{
    delete m_numerator;
    delete m_denominator;
}

void FractionElement::paint( QPainter& painter, AttributeManager* am )
{
    Q_UNUSED( am )
    // return if there is nothing to paint
    if( m_lineThickness == 0.0 )
        return;

    // paint the fraction line with the specified line width
    QPen pen;
    pen.setWidthF( m_lineThickness );
    painter.setPen( pen );
    painter.drawLine( m_fractionLine );
}

void FractionElement::layout( const AttributeManager* am )
{
    // get values of all attributes
    QString value = am->findValue( "linethickness", this );
    Length length;
    if(value == "thick")
        length.value = 2;
    else if(value == "medium")
        length.value = 1;
    else if(value == "thin")
        length.value = 0.5;
    else
        length = am->parseUnit( value, this );

    if(length.unit == Length::None)
        m_lineThickness = am->lineThickness(this) * length.value;
    else 
        m_lineThickness = am->lengthToPixels(length, this, "linethickness");

    // decide which layout is wanted
    if( am->boolOf( "bevelled", this ) )
    {
        layoutBevelledFraction( am );
        return;
    }

    qreal distY = am->layoutSpacing( this );
    Align numalign = am->alignOf( "numalign", this ); 
    Align denomalign = am->alignOf( "denomalign", this );

    // align the numerator and the denominator
    QPointF numeratorOrigin;
    QPointF denominatorOrigin( 0.0, m_numerator->height() + m_lineThickness + 2*distY );
    setWidth( qMax( m_numerator->width(), m_denominator->width() ) + m_lineThickness*2 );
    
    if( numalign == Right )
        numeratorOrigin.setX( width() - m_numerator->width() - m_lineThickness );
    else if( numalign == Center )
	numeratorOrigin.setX( ( width() - m_numerator->width() ) / 2 );

    if( denomalign == Right )
        denominatorOrigin.setX( width() - m_denominator->width() - m_lineThickness );
    else if( numalign == Center )
	denominatorOrigin.setX( ( width() - m_denominator->width() ) / 2 );

    m_numerator->setOrigin( numeratorOrigin );
    m_denominator->setOrigin( denominatorOrigin );

    // construct the fraction's line    
    qreal fractionLineY =  m_numerator->height() + m_lineThickness/2 + distY;
    m_fractionLine = QLineF( QPointF( m_lineThickness, fractionLineY ),
                             QPointF( width()-m_lineThickness, fractionLineY ) );

    setHeight( m_numerator->height() + m_denominator->height() +
               m_lineThickness + 2*distY );
    setBaseLine( denominatorOrigin.y() ); 
}

void FractionElement::layoutBevelledFraction( const AttributeManager* am )
{
    // the shown line should have a width that has 1/3 of the height
    // the line is heigher as the content by 2*thinmathspace = 2*borderY

    qreal borderY = am->layoutSpacing( this );
    setHeight( m_numerator->height() + m_denominator->height() + 2*borderY );
    setWidth( m_numerator->width() + m_denominator->width() + height()/3 );
    setBaseLine( height()/2 );

    m_numerator->setOrigin( QPointF( 0.0, borderY ) );
    m_denominator->setOrigin( QPointF( width()-m_denominator->width(),
                                       borderY+m_numerator->height() ) );
    m_fractionLine = QLineF( QPointF( m_numerator->width(), height() ),
                             QPointF( width()-m_denominator->width(), 0.0 ) );
}

const QList<BasicElement*> FractionElement::childElements() const
{
    QList<BasicElement*> list;
    list << m_numerator<<m_denominator;
    return list;
}

int FractionElement::endPosition() const {
    return 3;
}

// QLineF FractionElement::cursorLine(int position) const {
//     QPointF top=absoluteBoundingRect().topLeft();
//     QPointF bottom;
//     switch (position) {
// 	case 0:
// 	    top+=m_numerator->origin();
// 	    break;
// 	case 1:
// 	    top+=m_numerator->origin()+QPointF(m_numerator->width(),0.0);
// 	    break;
// 	case 2:
// 	    top+=m_denominator->origin();
// 	    break;
// 	case 3:
// 	    top+=m_denominator->origin()+QPointF(m_denominator->width(),0.0);
// 	    break;
//     }
//     if (position<=1) {
// 	bottom=top+QPointF(0.0,m_numerator->height());
//     }
//     else {
// 	bottom=top+QPointF(0.0,m_denominator->height());
//     }
//     return QLineF(top, bottom);
// }


QList< BasicElement* > FractionElement::elementsBetween ( int pos1, int pos2 ) const
{
    QList<BasicElement*> tmp;
    if (pos1==0 && pos2 >0) { 
        tmp.append(m_numerator);
    }
    if (pos1<3 && pos2==3) {
        tmp.append(m_denominator);
    }
    return tmp;
}


int FractionElement::positionOfChild(BasicElement* child) const {
    if (m_numerator==child){
	return 0;
    }
    else if (m_denominator==child) {
	return 2;
    }
    return -1;
}

bool FractionElement::moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor)  {
    if (newcursor.isSelecting()) {
        return false;
    } else {
        //TODO: How can I get the attribute of the Attributemanager here?
        // The movement should be different in the bevelled case
        //if (bevelled) {
        // return moveHorSituation(newcursor,oldcursor,0,1 )
        return moveVertSituation(newcursor,oldcursor,0,1);
    }
}

bool FractionElement::setCursorTo( FormulaCursor& cursor, QPointF point )
{
    //check if the point is above the fraction line, the origin is in the top left corner
    bool inNumerator=point.y() < (m_numerator->boundingRect().bottom() +  m_denominator->boundingRect().top())/2 ;
    if (cursor.isSelecting()) {
        return false;
    } else {
        if (point.x() > width()) {
            cursor.moveTo(this,inNumerator? 1 : 3);
            return true;
        }
        if (point.x() < 0) {
            cursor.moveTo(this,inNumerator? 0 : 2);
            return true;
        }
        if ( inNumerator ) {
            point-=m_numerator->origin();
            //TODO: maybe place it directly in the fraction if this fails
            return m_numerator->setCursorTo(cursor,point);
        } else {
            point-=m_denominator->origin();
            return m_denominator->setCursorTo(cursor,point);
        }
    }
}

bool FractionElement::replaceChild ( BasicElement* oldelement, BasicElement* newelement )
{
    //TODO: investigate, if we really need this
    if (newelement->elementType()==Row) {
        RowElement* newrow=static_cast<RowElement*>(newelement);
        if( oldelement == m_numerator ) {
            m_numerator = newrow;
            return true;
        } else if( oldelement == m_denominator ) {
            m_denominator = newrow;
            return true;
        }
    }
    return false;
}
   
QString FractionElement::attributesDefaultValue( const QString& attribute ) const
{
    if( attribute == "linethickness" )
        return "1";
    else if( attribute == "numalign" || attribute == "denomalign" )
        return "center";
    else if( attribute == "bevelled" )
        return "false";
    else
        return QString();
}

bool FractionElement::readMathMLContent( const KoXmlElement& parent )
{
    KoXmlElement tmp;
    int counter=0;
    forEachElement( tmp, parent ) {
        if (counter==0) {
            loadElement(tmp,&m_numerator);
        } else if (counter==1) {
            loadElement(tmp,&m_denominator);
        } else {
            kDebug(39001) << "Too many arguments to mfrac";
        }
        counter++;
    }
    if (counter<2) {
        kDebug(39001) << "Not enough arguments to mfrac";
    }
    return true;
}

void FractionElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    m_numerator->writeMathML( writer, ns );
    m_denominator->writeMathML( writer, ns );
}

ElementType FractionElement::elementType() const
{
    return Fraction;
}
