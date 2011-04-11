/* This file is part of the KDE project
 * Copyright (C) 2007-2009,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ArtisticTextShape.h"

#include <KoPathShape.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoPathShapeLoader.h>
#include <KoShapeBackground.h>

#include <KLocale>
#include <KDebug>

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QFont>

ArtisticTextShape::ArtisticTextShape()
    : m_path(0), m_startOffset(0.0), m_baselineOffset(0.0)
    , m_textAnchor( AnchorStart )
{
    setShapeId( ArtisticTextShapeID );
    cacheGlyphOutlines();
    updateSizeAndPosition();
}

ArtisticTextShape::~ArtisticTextShape()
{
    if (m_path) {
        m_path->removeDependee( this );
    }
}

void ArtisticTextShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    applyConversion( painter, converter );
    if( background() )
        background()->paint( painter, outline() );
}

void ArtisticTextShape::paintDecorations(QPainter &/*painter*/, const KoViewConverter &/*converter*/, const KoCanvasBase * /*canvas*/)
{
}

void ArtisticTextShape::saveOdf(KoShapeSavingContext &/*context*/) const
{
}

bool ArtisticTextShape::loadOdf(const KoXmlElement &/*element*/, KoShapeLoadingContext &/*context*/)
{
    return false;
}

QSizeF ArtisticTextShape::size() const
{
    if( m_ranges.isEmpty() )
        return nullBoundBox().size();
    else
        return outline().boundingRect().size();
}

void ArtisticTextShape::setSize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    if ( !oldSize.isNull() ) {
        qreal zoomX = newSize.width() / oldSize.width();
        qreal zoomY = newSize.height() / oldSize.height();
        QTransform matrix( zoomX, 0, 0, zoomY, 0, 0 );

        update();
        applyTransformation( matrix );
        update();
    }
    KoShape::setSize(newSize);
}

QPainterPath ArtisticTextShape::outline() const
{
    return m_outline;
}

QRectF ArtisticTextShape::nullBoundBox() const
{
    QFontMetrics metrics(defaultFont());
    return QRectF( QPointF(), QSizeF(metrics.averageCharWidth(), metrics.height()) );
}

QFont ArtisticTextShape::defaultFont() const
{
    return QFont("ComicSans", 20);
}

void ArtisticTextShape::createOutline()
{
    // reset relevant data
    m_outline = QPainterPath();
    m_charPositions.clear();
    m_charOffsets.clear();

    const int totalTextLength = plainText().length();

    // one more than the number of characters for position after the last character
    m_charPositions.resize(totalTextLength+1);

    // the character index within the text shape
    int globalCharIndex = 0;

    if( isOnPath() ) {
        // one more than the number of characters for offset after the last character
        m_charOffsets.resize(totalTextLength + 1);

        // the current character position
        QPointF charOffset(m_startOffset * m_baseline.length(), 0.0);
        // adjust starting character position to anchor point
        qreal totalTextWidth = 0.0;
        foreach (const ArtisticTextRange &range, m_ranges) {
            QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
            totalTextWidth += metrics.width(range.text());
        }
        qreal anchorPosition = 0.0;
        if( m_textAnchor == AnchorMiddle )
            anchorPosition = 0.5 * totalTextWidth;
        else if( m_textAnchor == AnchorEnd )
            anchorPosition = totalTextWidth;

        charOffset -= QPointF(anchorPosition, 0.0);

        QPointF pathPoint;
        QPointF offset;

        foreach (const ArtisticTextRange &range, m_ranges) {
            QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
            const QString textRange = range.text();
            const int localTextLength = textRange.length();

            const bool absoluteXOffset = range.xOffsetType() == ArtisticTextRange::AbsoluteOffset;
            const bool absoluteYOffset = range.yOffsetType() == ArtisticTextRange::AbsoluteOffset;

            for (int localCharIndex = 0; localCharIndex < localTextLength; ++localCharIndex, ++globalCharIndex) {
                QString actChar( textRange[localCharIndex] );

                // apply offset to character
                if (range.hasXOffset(localCharIndex)) {
                    if (absoluteXOffset)
                        charOffset.rx() = range.xOffset(localCharIndex);
                    else
                        charOffset.rx() += range.xOffset(localCharIndex);
                } else {
                    charOffset.rx() += offset.x();
                }
                if (range.hasYOffset(localCharIndex)) {
                    if (absoluteYOffset)
                        charOffset.ry() = range.yOffset(localCharIndex);
                    else
                        charOffset.ry() += range.yOffset(localCharIndex);
                } else {
                    charOffset.ry() += offset.y();
                }

                // get the percent value of the actual char position
                qreal t = m_baseline.percentAtLength( charOffset.x() );
                // first intialize with invalid position
                m_charOffsets[globalCharIndex] = -1;
                // are we beyond the baseline end?
                if (t >= 1.0) {
                    m_charPositions[globalCharIndex] = pathPoint;
                    continue;
                }

                // are we beyond the baseline start?
                if (t >= 0.0) {
                    // get the path point of the given path position
                    pathPoint = m_baseline.pointAtPercent( t );
                    // get the normalized position of the middle of the character
                    t = m_baseline.percentAtLength( charOffset.x() + 0.5 * metrics.width( actChar ) );
                }

                // save character offset as fraction of baseline length
                m_charOffsets[globalCharIndex] = m_baseline.percentAtLength(charOffset.x());
                // save character position as point
                m_charPositions[globalCharIndex] = pathPoint;

                // add the advance of the current character to the character position
                offset = QPointF(metrics.width( actChar ), 0.0);

                if (t <= 0.0) {
                    // if this is not the first character but our position is still
                    // zero or less, disable the previous character from display
                    if (globalCharIndex)
                        m_charOffsets[globalCharIndex-1] = -1;
                    continue;
                }

                // are we beyond the baseline end?
                if( t >= 1.0 )
                    break;

                // get the angle at the given path position
                const qreal angle = m_baseline.angleAtPercent( t );

                QTransform m;
                m.translate( pathPoint.x(), pathPoint.y() );
                m.rotate( 360. - angle );
                m.translate(0.0, charOffset.y());
                m_outline.addPath( m.map( m_charOutlines[globalCharIndex] ) );
            }
        }
        // save offset and position after last character
        m_charOffsets[globalCharIndex] = m_baseline.percentAtLength(charOffset.x());
        m_charPositions[globalCharIndex] = m_baseline.pointAtPercent(m_charOffsets[globalCharIndex]);
    } else {
        QPointF charPos(0, 0);
        QPointF offset(0, 0);
        foreach(const ArtisticTextRange &range, m_ranges) {
            QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
            const QString textRange = range.text();

            const bool absoluteXOffset = range.xOffsetType() == ArtisticTextRange::AbsoluteOffset;
            const bool absoluteYOffset = range.yOffsetType() == ArtisticTextRange::AbsoluteOffset;

            const int localTextLength = textRange.length();
            for(int localCharIndex = 0; localCharIndex < localTextLength; ++localCharIndex, ++globalCharIndex) {
                QString actChar(textRange[localCharIndex]);

                // apply offset to character
                if (range.hasXOffset(localCharIndex)) {
                    if (absoluteXOffset)
                        charPos.rx() = range.xOffset(localCharIndex);
                    else
                        charPos.rx() += range.xOffset(localCharIndex);
                } else {
                    charPos.rx() += offset.x();
                }
                if (range.hasYOffset(localCharIndex)) {
                    if (absoluteYOffset)
                        charPos.ry() = range.yOffset(localCharIndex);
                    else
                        charPos.ry() += range.yOffset(localCharIndex);
                } else {
                    charPos.ry() += offset.y();
                }

                QTransform m;
                m.translate(charPos.x(), charPos.y());
                m_outline.addPath(m.map(m_charOutlines[globalCharIndex]));
                // save character positon of current character
                m_charPositions[globalCharIndex] = charPos;
                // advance character position
                offset = QPointF(metrics.width(actChar), 0.0);
            }
        }
        m_charPositions[globalCharIndex] = charPos + offset;
    }
}

void ArtisticTextShape::setPlainText(const QString &newText)
{
    if( plainText() == newText )
        return;

    update();
    if (newText.isEmpty()) {
        // remove all text ranges
        m_ranges.clear();
    } else if (isEmpty()) {
        // create new text range
        m_ranges.append(ArtisticTextRange(newText, defaultFont()));
    } else {
        // set text to first range
        m_ranges.first().setText(newText);
        // remove all ranges except the first
        while(m_ranges.count() > 1)
            m_ranges.pop_back();
    }
    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
}

QString ArtisticTextShape::plainText() const
{
    QString allText;
    foreach(const ArtisticTextRange &range, m_ranges) {
        allText += range.text();
    }

    return allText;
}

QList<ArtisticTextRange> ArtisticTextShape::text() const
{
    return m_ranges;
}

bool ArtisticTextShape::isEmpty() const
{
    return m_ranges.isEmpty();
}

void ArtisticTextShape::clear()
{
    m_ranges.clear();
}

void ArtisticTextShape::setFont(const QFont &newFont)
{
    // no text
    if(isEmpty())
        return;

    const int rangeCount = m_ranges.count();
    // only one text range with the same font
    if(rangeCount == 1 && m_ranges.first().font() == newFont)
        return;

    update();

    ArtisticTextRange &firstRange = m_ranges.first();
    // set font to first range
    firstRange.setFont(newFont);
    // append all ranges to the first range
    for (int i = 1; i < rangeCount; ++i) {
        firstRange.append(m_ranges[1]);
        m_ranges.removeAt(1);
    }

    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();
}

QFont ArtisticTextShape::fontAt(int charIndex) const
{
    const int rangeIndex = indexOfChar(charIndex).first;
    if (rangeIndex < 0)
        return defaultFont();

    return m_ranges[rangeIndex].font();
}

void ArtisticTextShape::setStartOffset( qreal offset )
{
    if( m_startOffset == offset )
        return;

    update();
    m_startOffset = qBound(0.0, offset, 1.0);
    updateSizeAndPosition();
    update();
    notifyChanged();
}

qreal ArtisticTextShape::startOffset() const
{
    return m_startOffset;
}

qreal ArtisticTextShape::baselineOffset() const
{
    return m_baselineOffset;
}

void ArtisticTextShape::setTextAnchor( TextAnchor anchor )
{
    if( anchor == m_textAnchor )
        return;

    qreal totalTextWidth = 0.0;
    foreach (const ArtisticTextRange &range, m_ranges) {
        QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
        totalTextWidth += metrics.width(range.text());
    }

    qreal oldOffset = 0.0;
    if( m_textAnchor == AnchorMiddle )
        oldOffset = -0.5 * totalTextWidth;
    else if( m_textAnchor == AnchorEnd )
        oldOffset = -totalTextWidth;

    m_textAnchor = anchor;

    qreal newOffset = 0.0;
    if( m_textAnchor == AnchorMiddle )
        newOffset = -0.5 * totalTextWidth;
    else if( m_textAnchor == AnchorEnd )
        newOffset = -totalTextWidth;


    update();
    updateSizeAndPosition();
    if( ! isOnPath() ) {
        QTransform m;
        m.translate( newOffset-oldOffset, 0.0 );
        setTransformation( transformation() * m );
    }
    update();
    notifyChanged();
}

ArtisticTextShape::TextAnchor ArtisticTextShape::textAnchor() const
{
    return m_textAnchor;
}

bool ArtisticTextShape::putOnPath( KoPathShape * path )
{
    if( ! path )
        return false;

    if( path->outline().isEmpty() )
        return false;

    if( ! path->addDependee( this ) )
        return false;

    update();

    m_path = path;

    // use the paths outline converted to document coordinates as the baseline
    m_baseline = m_path->absoluteTransformation(0).map( m_path->outline() );

    // reset transformation
    setTransformation( QTransform() );
    updateSizeAndPosition();
    // move to correct position
    setAbsolutePosition( m_outlineOrigin, KoFlake::TopLeftCorner );
    update();

    return true;
}

bool ArtisticTextShape::putOnPath( const QPainterPath &path )
{
    if( path.isEmpty() )
        return false;

    update();
    if( m_path )
        m_path->removeDependee( this );
    m_path = 0;
    m_baseline = path;

    // reset transformation
    setTransformation( QTransform() );
    updateSizeAndPosition();
    // move to correct position
    setAbsolutePosition( m_outlineOrigin, KoFlake::TopLeftCorner );
    update();

    return true;
}

void ArtisticTextShape::removeFromPath()
{
    update();
    if( m_path )
        m_path->removeDependee( this );
    m_path = 0;
    m_baseline = QPainterPath();
    updateSizeAndPosition();
    update();
}

bool ArtisticTextShape::isOnPath() const
{
    return (m_path != 0 || ! m_baseline.isEmpty() );
}

ArtisticTextShape::LayoutMode ArtisticTextShape::layout() const
{
    if( m_path )
        return OnPathShape;
    else if( ! m_baseline.isEmpty() )
        return OnPath;
    else
        return Straight;
}


QPainterPath ArtisticTextShape::baseline() const
{
    return m_baseline;
}

KoPathShape * ArtisticTextShape::baselineShape() const
{
    return m_path;
}

QList<ArtisticTextRange> ArtisticTextShape::removeText(int charIndex, int charCount)
{
    QList<ArtisticTextRange> extractedRanges;

    if (!charCount)
        return extractedRanges;

    CharIndex charPos = indexOfChar(charIndex);
    if (charPos.first < 0)
        return extractedRanges;

    update();

    int extractedTextLength = 0;
    while(extractedTextLength < charCount) {
        ArtisticTextRange r = m_ranges[charPos.first].extract(charPos.second, charCount-extractedTextLength);
        extractedTextLength += r.text().length();
        extractedRanges.append(r);
        if (extractedTextLength == charCount)
            break;
        charPos.first++;
        if(charPos.first >= m_ranges.count())
            break;
        charPos.second = 0;
    }

    // now remove all empty ranges
    for (int i = charPos.first; i < m_ranges.count(); ++i) {
        if (m_ranges[i].text().isEmpty()) {
            m_ranges.removeAt(i);
            i--;
        }
    }

    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();

    return extractedRanges;
}

void ArtisticTextShape::insertText(int charIndex, const QString &str)
{
    CharIndex charPos = indexOfChar(charIndex);
    if (charIndex < 0 || isEmpty()) {
        // insert before first character
        charPos = CharIndex(0, 0);
    } else if (charIndex >= plainText().length()) {
        // insert after last character
        charPos = CharIndex(m_ranges.count()-1, m_ranges.last().text().length());
    }

    // check range index, just in case
    if (charPos.first < 0)
        return;

    update();

    m_ranges[charPos.first].insertText(charPos.second, str);

    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();
}

void ArtisticTextShape::insertText(int charIndex, const ArtisticTextRange &textRange)
{
    QList<ArtisticTextRange> ranges;
    ranges.append(textRange);
    insertText(charIndex, ranges);
}

void ArtisticTextShape::insertText(int charIndex, const QList<ArtisticTextRange> &textRanges)
{
    kDebug() << "1";

    CharIndex charPos = indexOfChar(charIndex);
    if (charIndex < 0 || isEmpty()) {
        // insert before first character
        charPos = CharIndex(0, 0);
    } else if (charIndex >= plainText().length()) {
        // insert after last character
        charPos = CharIndex(m_ranges.count()-1, m_ranges.last().text().length());
    }

    // check range index, just in case
    if (charPos.first < 0)
        return;

    update();

    ArtisticTextRange &hitRange = m_ranges[charPos.first];
    if (charPos.second == 0) {
        // insert ranges before the hit range
        foreach(const ArtisticTextRange &range, textRanges) {
            m_ranges.insert(charPos.first, range);
            charPos.first++;
        }
    } else if (charPos.second == hitRange.text().length()) {
        // insert ranges after the hit range
        foreach(const ArtisticTextRange &range, textRanges) {
            m_ranges.insert(charPos.first+1, range);
            charPos.first++;
        }
    } else {
        // insert ranges inside hit range
        ArtisticTextRange right = hitRange.extract(charPos.second, hitRange.text().length());
        m_ranges.insert(charPos.first+1, right);
        // now insert after the left part of hit range
        foreach(const ArtisticTextRange &range, textRanges) {
            m_ranges.insert(charPos.first+1, range);
            charPos.first++;
        }
    }

    // TODO: merge ranges with same style

    kDebug() << "2";
    cacheGlyphOutlines();
    kDebug() << "3";
    updateSizeAndPosition();
    kDebug() << "4";
    update();
    kDebug() << "5";
    notifyChanged();
}

void ArtisticTextShape::appendText(const QString &text)
{
    update();

    if (isEmpty()) {
        m_ranges.append(ArtisticTextRange(text, defaultFont()));
    } else {
        m_ranges.last().appendText(text);
    }

    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();
}

void ArtisticTextShape::appendText(const ArtisticTextRange &text)
{
    update();

    m_ranges.append(text);

    // TODO: merge ranges with same style

    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();

}

bool ArtisticTextShape::replaceText(int charIndex, int charCount, ArtisticTextRange &textRange)
{
    CharIndex charPos = indexOfChar(charIndex);
    if (charPos.first < 0 || !charCount)
        return false;

    qDebug() << "before insert:" << plainText();

    insertText(charIndex, textRange);

    qDebug() << "before remove:" << plainText();

    removeText(charIndex+textRange.text().length(), charCount);

    qDebug() << "after replace" << plainText();

    return true;
}

qreal ArtisticTextShape::charAngleAt(int charIndex) const
{
    if( isOnPath() ) {
        qreal t = m_charOffsets[qMin(charIndex, m_charOffsets.size())];
        return m_baseline.angleAtPercent( t );
    }

    return 0.0;
}

QPointF ArtisticTextShape::charPositionAt(int charIndex) const
{
    return m_charPositions.value(qMin(charIndex, m_charPositions.size()));
}

QRectF ArtisticTextShape::charExtentsAt(int charIndex) const
{
    CharIndex charPos = indexOfChar(charIndex);
    if (charIndex < 0 || isEmpty())
        charPos = CharIndex(0,0);
    else if(charPos.first < 0)
        charPos = CharIndex(m_ranges.count()-1, m_ranges.last().text().length()-1);

    const ArtisticTextRange &range = m_ranges.at(charPos.first);
    QFontMetrics metrics(range.font());
    int w = metrics.charWidth(range.text(), charPos.second);
    return QRectF( 0, 0, w, metrics.height() );
}

void ArtisticTextShape::updateSizeAndPosition( bool global )
{
    createOutline();

    QRectF bbox = m_outline.boundingRect();
    if( bbox.isEmpty() )
        bbox = nullBoundBox();

    // calculate the offset we have to apply to keep our position
    QPointF offset = m_outlineOrigin - bbox.topLeft();

    // cache topleft corner of baseline path
    m_outlineOrigin = bbox.topLeft();

    if( isOnPath() ) {
        // the outline position is in document coordinates
        // so we adjust our position
        QTransform m;
        m.translate( -offset.x(), -offset.y() );
        global ? applyAbsoluteTransformation( m ) : applyTransformation( m );
    } else {
        // the text outlines baseline is at 0,0
        m_baselineOffset = -m_outlineOrigin.y();
    }

    setSize( bbox.size() );

    // map outline to shape coordinate system
    QTransform normalizeMatrix;
    normalizeMatrix.translate( -m_outlineOrigin.x(), -m_outlineOrigin.y() );
    m_outline = normalizeMatrix.map( m_outline );
    const int charCount = m_charPositions.count();
    for (int i = 0; i < charCount; ++i)
        m_charPositions[i] = normalizeMatrix.map(m_charPositions[i]);
}

void ArtisticTextShape::cacheGlyphOutlines()
{
    m_charOutlines.clear();

    foreach(const ArtisticTextRange &range, m_ranges) {
        const QString text = range.text();
        const int textLength = text.length();
        for(int charIdx = 0; charIdx < textLength; ++charIdx) {
            QPainterPath charOutline;
            charOutline.addText(QPointF(), QFont(range.font(), &m_paintDevice), text[charIdx]);
            m_charOutlines.append(charOutline);
        }
    }
}

void ArtisticTextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    if( m_path && shape == m_path ) {
        if( type == KoShape::Deleted ) {
            // baseline shape was deleted
            m_path = 0;
        } else if (type == KoShape::ParentChanged && !shape->parent()) {
            // baseline shape was probably removed from the document
            m_path->removeDependee(this);
            m_path = 0;
        } else {
            update();
            // use the paths outline converted to document coordinates as the baseline
            m_baseline = m_path->absoluteTransformation(0).map( m_path->outline() );
            updateSizeAndPosition( true );
            update();
        }
    }
}

ArtisticTextShape::CharIndex ArtisticTextShape::indexOfChar(int charIndex) const
{
    if (isEmpty())
        return CharIndex(-1,-1);

    int rangeIndex = 0;
    int textLength = 0;
    foreach(const ArtisticTextRange &range, m_ranges) {
        const int rangeTextLength = range.text().length();
        if (static_cast<int>(charIndex) < textLength+rangeTextLength) {
            return CharIndex(rangeIndex, charIndex-textLength);
        }
        textLength += rangeTextLength;
        rangeIndex++;
    }

    return CharIndex(-1, -1);
}
