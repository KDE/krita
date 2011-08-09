/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>
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

#include "TokenElement.h"
#include "AttributeManager.h"
#include "FormulaCursor.h"
#include "Dictionary.h"
#include "GlyphElement.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <QPainter>
#include <kdebug.h>

TokenElement::TokenElement( BasicElement* parent ) : BasicElement( parent )
{
    m_stretchHorizontally = false;
    m_stretchVertically = false;
}

const QList<BasicElement*> TokenElement::childElements() const
{
    // only return the mglyph elements
    QList<BasicElement*> tmpList;
    foreach( GlyphElement* tmp, m_glyphs )
        tmpList << tmp;

    return tmpList;
}
void TokenElement::paint( QPainter& painter, AttributeManager* am )
{
    // set the painter to background color and paint it
    painter.setPen( am->colorOf( "mathbackground", this ) );
    painter.setBrush( QBrush( painter.pen().color() ) );
    painter.drawRect( QRectF( 0.0, 0.0, width(), height() ) );

    // set the painter to foreground color and paint the text in the content path
    QColor color = am->colorOf( "mathcolor", this );
    if (!color.isValid())
        color = am->colorOf( "color", this );

    painter.translate( m_xoffset, baseLine() );
    if(m_stretchHorizontally || m_stretchVertically)
        painter.scale(width() / m_originalSize.width(), height() / m_originalSize.height());

    painter.setPen( color );
    painter.setBrush( QBrush( color ) );
    painter.drawPath( m_contentPath );
}

int TokenElement::endPosition() const
{
    return m_rawString.length();
}

void TokenElement::layout( const AttributeManager* am )
{
    m_offsets.erase(m_offsets.begin(),m_offsets.end());
    m_offsets << 0.0;
    // Query the font to use
    m_font = am->font( this );
    QFontMetricsF fm(m_font);

    // save the token in an empty path
    m_contentPath = QPainterPath();

    /* Current bounding box.  Note that the left can be negative, for italics etc */
    QRectF boundingrect;
    if(m_glyphs.isEmpty()) {//optimize for the common case
        boundingrect = renderToPath(m_rawString, m_contentPath);
        for (int j = 0; j < m_rawString.length(); ++j) {
                m_offsets.append(fm.width(m_rawString.left(j+1)));
        }
     } else {
        // replace all the object replacement characters with glyphs
        // We have to keep track of the bounding box at all times
        QString chunk;
        int counter = 0;
        for( int i = 0; i < m_rawString.length(); i++ ) {
            if( m_rawString[ i ] != QChar::ObjectReplacementCharacter )
                chunk.append( m_rawString[ i ] );
            else {
                m_contentPath.moveTo(boundingrect.right(), 0);
                QRectF newbox = renderToPath( chunk, m_contentPath );
                boundingrect.setRight( boundingrect.right() + newbox.right());
                boundingrect.setTop( qMax(boundingrect.top(), newbox.top()));
                boundingrect.setBottom( qMax(boundingrect.bottom(), newbox.bottom()));
                qreal glyphoffset = m_offsets.last();
                for (int j = 0; j < chunk.length(); ++j) {
                    m_offsets << fm.width(chunk.left(j+1)) + glyphoffset;
                }
                m_contentPath.moveTo(boundingrect.right(), 0);
                newbox = m_glyphs[ counter ]->renderToPath( QString(), m_contentPath );
                boundingrect.setRight( boundingrect.right() + newbox.right());
                boundingrect.setTop( qMax(boundingrect.top(), newbox.top()));
                boundingrect.setBottom( qMax(boundingrect.bottom(), newbox.bottom()));
                m_offsets.append(newbox.width() + m_offsets.last());
                counter++;
                chunk.clear();
            }
        }
        if( !chunk.isEmpty() ) {
            m_contentPath.moveTo(boundingrect.right(), 0);
            QRectF newbox = renderToPath( chunk, m_contentPath );
            boundingrect.setRight( boundingrect.right() + newbox.right());
            boundingrect.setTop( qMax(boundingrect.top(), newbox.top()));
            boundingrect.setBottom( qMax(boundingrect.bottom(), newbox.bottom()));
//             qreal glyphoffset = m_offsets.last();
            for (int j = 0; j < chunk.length(); ++j) {
                m_offsets << fm.width(chunk.left(j+1)) + m_offsets.last();
            }
        }
    }
    //FIXME: This is only a temporary solution
    boundingrect=m_contentPath.boundingRect();
    m_offsets.removeLast();
    m_offsets.append(m_contentPath.boundingRect().right());
    //The left side may be negative, because of italised letters etc. we need to adjust for this when painting
    //The qMax is just incase.  The bounding box left should never be >0
    m_xoffset = qMax(-boundingrect.left(), (qreal)0.0);
    // As the text is added to (0,0) the baseline equals the top edge of the
    // elements bounding rect, while translating it down the text's baseline moves too
    setBaseLine( -boundingrect.y() ); // set baseline accordingly
    setWidth( boundingrect.right() + m_xoffset );
    setHeight( boundingrect.height() );
    m_originalSize = QSizeF(width(), height());
}

bool TokenElement::insertChild( int position, BasicElement* child )
{
    Q_UNUSED( position)
    Q_UNUSED( child )
    //if( child && child->elementType() == Glyph ) {
    //m_rawString.insert( QChar( QChar::ObjectReplacementCharacter ) );
    //    m_glyphs.insert();
    //    return false;
    //} else {
    return false;
    //}
}


void TokenElement::insertGlyphs ( int position, QList< GlyphElement* > glyphs )
{
    for (int i=0; i < glyphs.length(); ++i) {
        m_glyphs.insert(position+i,glyphs[i]);
    }
}


bool TokenElement::insertText ( int position, const QString& text )
{
    m_rawString.insert (position,text);
    return true;
}


QList< GlyphElement* > TokenElement::glyphList ( int position, int length )
{
    QList<GlyphElement*> tmp;
    //find out, how many glyphs we have
    int counter=0;
    for (int i=position; i<position+length; ++i) {
        if (m_rawString[ position ] == QChar::ObjectReplacementCharacter) {
            counter++;
        }
    }
    int start=0;
    //find out where we should start removing glyphs
    if (counter>0) {
        for (int i=0; i<position; ++i) {
            if (m_rawString[position] == QChar::ObjectReplacementCharacter) {
                start++;
            }
        }
    }
    for (int i=start; i<start+counter; ++i) {
        tmp.append(m_glyphs.at(i));
    }
    return tmp;
}


int TokenElement::removeText ( int position, int length )
{
    //find out, how many glyphs we have
    int counter=0;
    for (int i=position; i<position+length; ++i) {
        if (m_rawString[ position ] == QChar::ObjectReplacementCharacter) {
            counter++;
        }
    }
    
    int start=0;
    //find out where we should start removing glyphs
    if (counter>0) {
        for (int i=0; i<position; ++i) {
            if (m_rawString[position] == QChar::ObjectReplacementCharacter) {
                start++;
            }
        }
    } 
    for (int i=start; i<start+counter; ++i) {
        m_glyphs.removeAt(i);
    }
    m_rawString.remove(position,length);
    return start;
}

bool TokenElement::setCursorTo(FormulaCursor& cursor, QPointF point) {
    int i = 0;
    cursor.setCurrentElement(this);
    if (cursorOffset(endPosition())<point.x()) {
        cursor.setPosition(endPosition());
        return true;
    }
    //Find the letter we clicked on
    for( i = 1; i < endPosition(); ++i ) {
        if (point.x() < cursorOffset(i)) {
            break;
        }
    }
    
    //Find out, if we should place the cursor before or after the character
    if ((point.x()-cursorOffset(i-1))<(cursorOffset(i)-point.x())) {	
        --i;
    }
    cursor.setPosition(i);
    return true;
}


QLineF TokenElement::cursorLine(int position) const
{
    // inside tokens let the token calculate the cursor x offset
    qreal tmp = cursorOffset( position );
    QPointF top = absoluteBoundingRect().topLeft() + QPointF( tmp, 0 );
    QPointF bottom = top + QPointF( 0.0,height() );
    return QLineF(top,bottom);
}

bool TokenElement::acceptCursor( const FormulaCursor& cursor )
{
    Q_UNUSED( cursor )
    return true;
}

bool TokenElement::moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor) {
    Q_UNUSED( oldcursor )
    if ((newcursor.direction()==MoveUp) ||
        (newcursor.direction()==MoveDown) ||
        (newcursor.isHome() && newcursor.direction()==MoveLeft) ||
        (newcursor.isEnd() && newcursor.direction()==MoveRight) ) {
        return false;
    }
    switch( newcursor.direction() ) {
    case MoveLeft:
        newcursor+=-1;
        break;
    case MoveRight:
        newcursor+=1;
        break;
    default:
        break;
    }
    return true;
}


qreal TokenElement::cursorOffset( const int position) const
{
    return m_offsets[position]+m_xoffset;
}

QFont TokenElement::font() const
{
    return m_font;
}


void TokenElement::setText ( const QString& text )
{
    removeText(0,m_rawString.length());
    insertText(0,text);
}

const QString& TokenElement::text()
{
    return m_rawString;
}


bool TokenElement::readMathMLContent( const KoXmlElement& element )
{
    // iterate over all child elements ( possible embedded glyphs ) and put the text
    // content in the m_rawString and mark glyph positions with
    // QChar::ObjectReplacementCharacter
    GlyphElement* tmpGlyph;
    KoXmlNode node = element.firstChild();
    while( !node.isNull() ) {
        if( node.isElement() && node.toElement().tagName() == "mglyph" ) {
            tmpGlyph = new GlyphElement( this );
            m_rawString.append( QChar( QChar::ObjectReplacementCharacter ) );
            tmpGlyph->readMathML( node.toElement() );
            m_glyphs.append(tmpGlyph);
        }
        else if( node.isElement() )
            return false;
        /*
        else if (node.isEntityReference()) {
            Dictionary dict;
            m_rawString.append( dict.mapEntity( node.nodeName() ) );
        }
        */
        else {
            m_rawString.append( node.toText().data() );
        }

        node = node.nextSibling();
    }
    m_rawString = m_rawString.simplified();
    return true;
}

void TokenElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    // split the m_rawString into text content chunks that are divided by glyphs 
    // which are represented as ObjectReplacementCharacter and write each chunk
    QStringList tmp = m_rawString.split( QChar( QChar::ObjectReplacementCharacter ) );
    for ( int i = 0; i < tmp.count(); i++ ) {
        if( m_rawString.startsWith( QChar( QChar::ObjectReplacementCharacter ) ) ) {
            m_glyphs[ i ]->writeMathML( writer, ns );
            if (i + 1 < tmp.count()) {
                writer->addTextNode( tmp[ i ] );
            }
        }
        else {
            writer->addTextNode( tmp[ i ] );
            if (i + 1 < tmp.count()) {
                m_glyphs[ i ]->writeMathML( writer, ns );
            }
        }
    }
}

const QString TokenElement::writeElementContent() const
{
    return m_rawString;
}
