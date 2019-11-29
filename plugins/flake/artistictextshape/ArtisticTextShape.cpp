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
#include "ArtisticTextLoadingContext.h"

#include <KoPathShape.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoPathShapeLoader.h>
#include <KoShapeBackground.h>
#include <KoEmbeddedDocumentSaver.h>
#include <SvgSavingContext.h>
#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>
#include <SvgStyleParser.h>
#include <SvgWriter.h>
#include <SvgStyleWriter.h>
#include <KisQPainterStateSaver.h>

#include <klocalizedstring.h>

#include <QDebug>
#include <QBuffer>
#include <QPen>
#include <QPainter>
#include <QFont>

ArtisticTextShape::ArtisticTextShape()
    : m_path(0)
    , m_startOffset(0.0)
    , m_textAnchor(AnchorStart)
    , m_textUpdateCounter(0)
    , m_defaultFont("ComicSans", 20)
{
    setShapeId(ArtisticTextShapeID);
    cacheGlyphOutlines();
    updateSizeAndPosition();
}

ArtisticTextShape::~ArtisticTextShape()
{
    if (m_path) {
        m_path->removeDependee(this);
    }
}

KoShape *ArtisticTextShape::cloneShape() const
{
    ArtisticTextShape *clone = new ArtisticTextShape();
    clone->m_ranges = m_ranges;
    if (m_path) {
        clone->m_path = static_cast<KoPathShape*>(m_path->cloneShape());
    }
    clone->m_charOutlines = m_charOutlines;
    clone->m_startOffset = m_startOffset;
    clone->m_outlineOrigin = m_outlineOrigin;
    clone->m_outline = m_outline;
    clone->m_baseline = m_baseline;
    clone->m_textAnchor = m_textAnchor;
    clone->m_charOffsets = m_charOffsets;
    clone->m_charPositions = m_charPositions;
    clone->m_textUpdateCounter = m_textUpdateCounter;
    clone->m_defaultFont = m_defaultFont;
    return clone;
}

void ArtisticTextShape::paint(QPainter &painter, KoShapePaintingContext &paintContext)
{
    KisQPainterStateSaver saver(&painter);

    if (background()) {
        background()->paint(painter, paintContext, outline());
    }
}

void ArtisticTextShape::saveOdf(KoShapeSavingContext &context) const
{
    SvgWriter svgWriter(QList<KoShape *>() << const_cast<ArtisticTextShape *>(this));
    QByteArray fileContent;
    QBuffer fileContentDevice(&fileContent);
    if (!fileContentDevice.open(QIODevice::WriteOnly)) {
        return;
    }

    if (!svgWriter.save(fileContentDevice, size())) {
        qWarning() << "Could not write svg content";
        return;
    }

    const QString fileName = context.embeddedSaver().getFilename("SvgImages/Image");
    const QString mimeType = "image/svg+xml";

    context.xmlWriter().startElement("draw:frame");
    context.embeddedSaver().embedFile(context.xmlWriter(), "draw:image", fileName, mimeType.toLatin1(), fileContent);
    context.xmlWriter().endElement(); // draw:frame
}

bool ArtisticTextShape::loadOdf(const KoXmlElement &/*element*/, KoShapeLoadingContext &/*context*/)
{
    return false;
}

QSizeF ArtisticTextShape::size() const
{
    if (m_ranges.isEmpty()) {
        return nullBoundBox().size();
    } else {
        return outline().boundingRect().size();
    }
}

void ArtisticTextShape::setSize(const QSizeF &newSize)
{
    QSizeF oldSize = size();
    if (!oldSize.isNull()) {
        qreal zoomX = newSize.width() / oldSize.width();
        qreal zoomY = newSize.height() / oldSize.height();
        QTransform matrix(zoomX, 0, 0, zoomY, 0, 0);

        update();
        applyTransformation(matrix);
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
    QPointF tl(0.0, -metrics.ascent());
    QPointF br(metrics.averageCharWidth(), metrics.descent());
    return QRectF(tl, br);
}

QFont ArtisticTextShape::defaultFont() const
{
    return m_defaultFont;
}

qreal baselineShiftForFontSize(const ArtisticTextRange &range, qreal fontSize)
{
    switch (range.baselineShift()) {
    case ArtisticTextRange::Sub:
        return fontSize / 3.; // taken from wikipedia
    case ArtisticTextRange::Super:
        return -fontSize / 3.; // taken from wikipedia
    case ArtisticTextRange::Percent:
        return range.baselineShiftValue() * fontSize;
    case ArtisticTextRange::Length:
        return range.baselineShiftValue();
    default:
        return 0.0;
    }
}

QVector<QPointF> ArtisticTextShape::calculateAbstractCharacterPositions()
{
    const int totalTextLength = plainText().length();

    QVector<QPointF> charPositions;

    // one more than the number of characters for position after the last character
    charPositions.resize(totalTextLength + 1);

    // the character index within the text shape
    int globalCharIndex = 0;

    QPointF charPos(0, 0);
    QPointF advance(0, 0);

    const bool attachedToPath = isOnPath();

    Q_FOREACH (const ArtisticTextRange &range, m_ranges) {
        QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
        const QString textRange = range.text();
        const qreal letterSpacing = range.letterSpacing();
        const int localTextLength = textRange.length();

        const bool absoluteXOffset = range.xOffsetType() == ArtisticTextRange::AbsoluteOffset;
        const bool absoluteYOffset = range.yOffsetType() == ArtisticTextRange::AbsoluteOffset;

        // set baseline shift
        const qreal baselineShift = baselineShiftForFontSize(range, defaultFont().pointSizeF());

        for (int localCharIndex = 0; localCharIndex < localTextLength; ++localCharIndex, ++globalCharIndex) {
            // apply offset to character
            if (range.hasXOffset(localCharIndex)) {
                if (absoluteXOffset) {
                    charPos.rx() = range.xOffset(localCharIndex);
                } else {
                    charPos.rx() += range.xOffset(localCharIndex);
                }
            } else {
                charPos.rx() += advance.x();
            }
            if (range.hasYOffset(localCharIndex)) {
                if (absoluteYOffset) {
                    // when attached to a path, absolute y-offsets are ignored
                    if (!attachedToPath) {
                        charPos.ry() = range.yOffset(localCharIndex);
                    }
                } else {
                    charPos.ry() += range.yOffset(localCharIndex);
                }
            } else {
                charPos.ry() += advance.y();
            }

            // apply baseline shift
            charPos.ry() += baselineShift;

            // save character position of current character
            charPositions[globalCharIndex] = charPos;
            // advance character position
            advance = QPointF(metrics.width(textRange[localCharIndex]) + letterSpacing, 0.0);

            charPos.ry() -= baselineShift;
        }
    }
    charPositions[globalCharIndex] = charPos + advance;

    return charPositions;
}

void ArtisticTextShape::createOutline()
{
    // reset relevant data
    m_outline = QPainterPath();
    m_charPositions.clear();
    m_charOffsets.clear();

    // calculate character positions in baseline coordinates
    m_charPositions = calculateAbstractCharacterPositions();

    // the character index within the text shape
    int globalCharIndex = 0;

    if (isOnPath()) {
        // one more than the number of characters for offset after the last character
        m_charOffsets.insert(0, m_charPositions.size(), -1);
        // the current character position
        qreal startCharOffset = m_startOffset * m_baseline.length();
        // calculate total text width
        qreal totalTextWidth = 0.0;
        foreach (const ArtisticTextRange &range, m_ranges) {
            QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
            totalTextWidth += metrics.width(range.text());
        }
        // adjust starting character position to anchor point
        if (m_textAnchor == AnchorMiddle) {
            startCharOffset -= 0.5 * totalTextWidth;
        } else if (m_textAnchor == AnchorEnd) {
            startCharOffset -= totalTextWidth;
        }

        QPointF pathPoint;
        qreal rotation = 0.0;
        qreal charOffset;

        foreach (const ArtisticTextRange &range, m_ranges) {
            QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
            const QString localText = range.text();
            const int localTextLength = localText.length();

            for (int localCharIndex = 0; localCharIndex < localTextLength; ++localCharIndex, ++globalCharIndex) {
                QPointF charPos = m_charPositions[globalCharIndex];

                // apply advance along baseline
                charOffset = startCharOffset + charPos.x();

                const qreal charMidPoint = charOffset + 0.5 * metrics.width(localText[localCharIndex]);
                // get the normalized position of the middle of the character
                const qreal midT = m_baseline.percentAtLength(charMidPoint);
                // is the character midpoint beyond the baseline ends?
                if (midT <= 0.0 || midT >= 1.0) {
                    if (midT >= 1.0) {
                        pathPoint = m_baseline.pointAtPercent(1.0);
                        for (int i = globalCharIndex; i < m_charPositions.size(); ++i) {
                            m_charPositions[i] = pathPoint;
                            m_charOffsets[i] = 1.0;
                        }
                        break;
                    } else {
                        m_charPositions[globalCharIndex] = m_baseline.pointAtPercent(0.0);
                        m_charOffsets[globalCharIndex] = 0.0;
                        continue;
                    }
                }
                // get the percent value of the actual char position
                qreal t = m_baseline.percentAtLength(charOffset);

                // get the path point of the given path position
                pathPoint = m_baseline.pointAtPercent(t);

                // save character offset as fraction of baseline length
                m_charOffsets[globalCharIndex] = m_baseline.percentAtLength(charOffset);
                // save character position as point
                m_charPositions[globalCharIndex] = pathPoint;

                // get the angle at the given path position
                const qreal angle = m_baseline.angleAtPercent(midT);
                if (range.hasRotation(localCharIndex)) {
                    rotation = range.rotation(localCharIndex);
                }

                QTransform m;
                m.translate(pathPoint.x(), pathPoint.y());
                m.rotate(360. - angle + rotation);
                m.translate(0.0, charPos.y());
                m_outline.addPath(m.map(m_charOutlines[globalCharIndex]));
            }
        }
        // save offset and position after last character
        m_charOffsets[globalCharIndex] = m_baseline.percentAtLength(startCharOffset + m_charPositions[globalCharIndex].x());
        m_charPositions[globalCharIndex] = m_baseline.pointAtPercent(m_charOffsets[globalCharIndex]);
    } else {
        qreal rotation = 0.0;
        Q_FOREACH (const ArtisticTextRange &range, m_ranges) {
            const QString textRange = range.text();
            const int localTextLength = textRange.length();
            for (int localCharIndex = 0; localCharIndex < localTextLength; ++localCharIndex, ++globalCharIndex) {
                const QPointF &charPos = m_charPositions[globalCharIndex];
                if (range.hasRotation(localCharIndex)) {
                    rotation = range.rotation(localCharIndex);
                }

                QTransform m;
                m.translate(charPos.x(), charPos.y());
                m.rotate(rotation);
                m_outline.addPath(m.map(m_charOutlines[globalCharIndex]));
            }
        }
    }
}

void ArtisticTextShape::setPlainText(const QString &newText)
{
    if (plainText() == newText) {
        return;
    }

    beginTextUpdate();

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
        while (m_ranges.count() > 1) {
            m_ranges.pop_back();
        }
    }

    finishTextUpdate();
}

QString ArtisticTextShape::plainText() const
{
    QString allText;
    Q_FOREACH (const ArtisticTextRange &range, m_ranges) {
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
    beginTextUpdate();
    m_ranges.clear();
    finishTextUpdate();
}

void ArtisticTextShape::setFont(const QFont &newFont)
{
    // no text
    if (isEmpty()) {
        return;
    }

    const int rangeCount = m_ranges.count();
    // only one text range with the same font
    if (rangeCount == 1 && m_ranges.first().font() == newFont) {
        return;
    }

    beginTextUpdate();

    // set font on ranges
    for (int i = 0; i < rangeCount; ++i) {
        m_ranges[i].setFont(newFont);
    }

    m_defaultFont = newFont;

    finishTextUpdate();
}

void ArtisticTextShape::setFont(int charIndex, int charCount, const QFont &font)
{
    if (isEmpty() || charCount <= 0) {
        return;
    }

    if (charIndex == 0 && charCount == plainText().length()) {
        setFont(font);
        return;
    }

    CharIndex charPos = indexOfChar(charIndex);
    if (charPos.first < 0 || charPos.first >= m_ranges.count()) {
        return;
    }

    beginTextUpdate();

    int remainingCharCount = charCount;
    while (remainingCharCount > 0) {
        ArtisticTextRange &currRange = m_ranges[charPos.first];
        // does this range have a different font ?
        if (currRange.font() != font) {
            if (charPos.second == 0 && currRange.text().length() < remainingCharCount) {
                // set font on all characters of this range
                currRange.setFont(font);
                remainingCharCount -= currRange.text().length();
            } else {
                ArtisticTextRange changedRange = currRange.extract(charPos.second, remainingCharCount);
                changedRange.setFont(font);
                if (charPos.second == 0) {
                    m_ranges.insert(charPos.first, changedRange);
                } else if (charPos.second >= currRange.text().length()) {
                    m_ranges.insert(charPos.first + 1, changedRange);
                } else {
                    ArtisticTextRange remainingRange = currRange.extract(charPos.second);
                    m_ranges.insert(charPos.first + 1, changedRange);
                    m_ranges.insert(charPos.first + 2, remainingRange);
                }
                charPos.first++;
                remainingCharCount -= changedRange.text().length();
            }
        }
        charPos.first++;
        if (charPos.first >= m_ranges.count()) {
            break;
        }
        charPos.second = 0;
    }

    finishTextUpdate();
}

QFont ArtisticTextShape::fontAt(int charIndex) const
{
    if (isEmpty()) {
        return defaultFont();
    }
    if (charIndex < 0) {
        return m_ranges.first().font();
    }
    const int rangeIndex = indexOfChar(charIndex).first;
    if (rangeIndex < 0) {
        return m_ranges.last().font();
    }

    return m_ranges[rangeIndex].font();
}

void ArtisticTextShape::setStartOffset(qreal offset)
{
    if (m_startOffset == offset) {
        return;
    }

    update();
    m_startOffset = qBound<qreal>(0.0, offset, 1.0);
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
    return m_charPositions.value(0).y();
}

void ArtisticTextShape::setTextAnchor(TextAnchor anchor)
{
    if (anchor == m_textAnchor) {
        return;
    }

    qreal totalTextWidth = 0.0;
    foreach (const ArtisticTextRange &range, m_ranges) {
        QFontMetricsF metrics(QFont(range.font(), &m_paintDevice));
        totalTextWidth += metrics.width(range.text());
    }

    qreal oldOffset = 0.0;
    if (m_textAnchor == AnchorMiddle) {
        oldOffset = -0.5 * totalTextWidth;
    } else if (m_textAnchor == AnchorEnd) {
        oldOffset = -totalTextWidth;
    }

    m_textAnchor = anchor;

    qreal newOffset = 0.0;
    if (m_textAnchor == AnchorMiddle) {
        newOffset = -0.5 * totalTextWidth;
    } else if (m_textAnchor == AnchorEnd) {
        newOffset = -totalTextWidth;
    }

    update();
    updateSizeAndPosition();
    if (! isOnPath()) {
        QTransform m;
        m.translate(newOffset - oldOffset, 0.0);
        setTransformation(transformation() * m);
    }
    update();
    notifyChanged();
}

ArtisticTextShape::TextAnchor ArtisticTextShape::textAnchor() const
{
    return m_textAnchor;
}

bool ArtisticTextShape::putOnPath(KoPathShape *path)
{
    if (! path) {
        return false;
    }

    if (path->outline().isEmpty()) {
        return false;
    }

    if (! path->addDependee(this)) {
        return false;
    }

    update();

    m_path = path;

    // use the paths outline converted to document coordinates as the baseline
    m_baseline = m_path->absoluteTransformation().map(m_path->outline());

    // reset transformation
    setTransformation(QTransform());
    updateSizeAndPosition();
    // move to correct position
    setAbsolutePosition(m_outlineOrigin, KoFlake::TopLeft);
    update();

    return true;
}

bool ArtisticTextShape::putOnPath(const QPainterPath &path)
{
    if (path.isEmpty()) {
        return false;
    }

    update();
    if (m_path) {
        m_path->removeDependee(this);
    }
    m_path = 0;
    m_baseline = path;

    // reset transformation
    setTransformation(QTransform());
    updateSizeAndPosition();
    // move to correct position
    setAbsolutePosition(m_outlineOrigin, KoFlake::TopLeft);
    update();

    return true;
}

void ArtisticTextShape::removeFromPath()
{
    update();
    if (m_path) {
        m_path->removeDependee(this);
    }
    m_path = 0;
    m_baseline = QPainterPath();
    updateSizeAndPosition();
    update();
}

bool ArtisticTextShape::isOnPath() const
{
    return (m_path != 0 || ! m_baseline.isEmpty());
}

ArtisticTextShape::LayoutMode ArtisticTextShape::layout() const
{
    if (m_path) {
        return OnPathShape;
    } else if (! m_baseline.isEmpty()) {
        return OnPath;
    } else {
        return Straight;
    }
}

QPainterPath ArtisticTextShape::baseline() const
{
    return m_baseline;
}

KoPathShape *ArtisticTextShape::baselineShape() const
{
    return m_path;
}

QList<ArtisticTextRange> ArtisticTextShape::removeText(int charIndex, int charCount)
{
    QList<ArtisticTextRange> extractedRanges;
    if (!charCount) {
        return extractedRanges;
    }

    if (charIndex == 0 && charCount >= plainText().length()) {
        beginTextUpdate();
        extractedRanges = m_ranges;
        m_ranges.clear();
        finishTextUpdate();
        return extractedRanges;
    }

    CharIndex charPos = indexOfChar(charIndex);
    if (charPos.first < 0 || charPos.first >= m_ranges.count()) {
        return extractedRanges;
    }

    beginTextUpdate();

    int extractedTextLength = 0;
    while (extractedTextLength < charCount) {
        ArtisticTextRange r = m_ranges[charPos.first].extract(charPos.second, charCount - extractedTextLength);
        extractedTextLength += r.text().length();
        extractedRanges.append(r);
        if (extractedTextLength == charCount) {
            break;
        }
        charPos.first++;
        if (charPos.first >= m_ranges.count()) {
            break;
        }
        charPos.second = 0;
    }

    // now remove all empty ranges
    const int rangeCount = m_ranges.count();
    for (int i = charPos.first; i < rangeCount; ++i) {
        if (m_ranges[charPos.first].text().isEmpty()) {
            m_ranges.removeAt(charPos.first);
        }
    }

    finishTextUpdate();

    return extractedRanges;
}

QList<ArtisticTextRange> ArtisticTextShape::copyText(int charIndex, int charCount)
{
    QList<ArtisticTextRange> extractedRanges;
    if (!charCount) {
        return extractedRanges;
    }

    CharIndex charPos = indexOfChar(charIndex);
    if (charPos.first < 0 || charPos.first >= m_ranges.count()) {
        return extractedRanges;
    }

    int extractedTextLength = 0;
    while (extractedTextLength < charCount) {
        ArtisticTextRange copy = m_ranges[charPos.first];
        ArtisticTextRange r = copy.extract(charPos.second, charCount - extractedTextLength);
        extractedTextLength += r.text().length();
        extractedRanges.append(r);
        if (extractedTextLength == charCount) {
            break;
        }
        charPos.first++;
        if (charPos.first >= m_ranges.count()) {
            break;
        }
        charPos.second = 0;
    }

    return extractedRanges;
}

void ArtisticTextShape::insertText(int charIndex, const QString &str)
{
    if (isEmpty()) {
        appendText(str);
        return;
    }

    CharIndex charPos = indexOfChar(charIndex);
    if (charIndex < 0) {
        // insert before first character
        charPos = CharIndex(0, 0);
    } else if (charIndex >= plainText().length()) {
        // insert after last character
        charPos = CharIndex(m_ranges.count() - 1, m_ranges.last().text().length());
    }

    // check range index, just in case
    if (charPos.first < 0) {
        return;
    }

    beginTextUpdate();

    m_ranges[charPos.first].insertText(charPos.second, str);

    finishTextUpdate();
}

void ArtisticTextShape::insertText(int charIndex, const ArtisticTextRange &textRange)
{
    QList<ArtisticTextRange> ranges;
    ranges.append(textRange);
    insertText(charIndex, ranges);
}

void ArtisticTextShape::insertText(int charIndex, const QList<ArtisticTextRange> &textRanges)
{
    if (isEmpty()) {
        beginTextUpdate();
        m_ranges = textRanges;
        finishTextUpdate();
        return;
    }

    CharIndex charPos = indexOfChar(charIndex);
    if (charIndex < 0) {
        // insert before first character
        charPos = CharIndex(0, 0);
    } else if (charIndex >= plainText().length()) {
        // insert after last character
        charPos = CharIndex(m_ranges.count() - 1, m_ranges.last().text().length());
    }

    // check range index, just in case
    if (charPos.first < 0) {
        return;
    }

    beginTextUpdate();

    ArtisticTextRange &hitRange = m_ranges[charPos.first];
    if (charPos.second == 0) {
        // insert ranges before the hit range
        Q_FOREACH (const ArtisticTextRange &range, textRanges) {
            m_ranges.insert(charPos.first, range);
            charPos.first++;
        }
    } else if (charPos.second == hitRange.text().length()) {
        // insert ranges after the hit range
        Q_FOREACH (const ArtisticTextRange &range, textRanges) {
            m_ranges.insert(charPos.first + 1, range);
            charPos.first++;
        }
    } else {
        // insert ranges inside hit range
        ArtisticTextRange right = hitRange.extract(charPos.second, hitRange.text().length());
        m_ranges.insert(charPos.first + 1, right);
        // now insert after the left part of hit range
        Q_FOREACH (const ArtisticTextRange &range, textRanges) {
            m_ranges.insert(charPos.first + 1, range);
            charPos.first++;
        }
    }

    // TODO: merge ranges with same style

    finishTextUpdate();
}

void ArtisticTextShape::appendText(const QString &text)
{
    beginTextUpdate();

    if (isEmpty()) {
        m_ranges.append(ArtisticTextRange(text, defaultFont()));
    } else {
        m_ranges.last().appendText(text);
    }

    finishTextUpdate();
}

void ArtisticTextShape::appendText(const ArtisticTextRange &text)
{
    beginTextUpdate();

    m_ranges.append(text);

    // TODO: merge ranges with same style

    finishTextUpdate();
}

bool ArtisticTextShape::replaceText(int charIndex, int charCount, const ArtisticTextRange &textRange)
{
    QList<ArtisticTextRange> ranges;
    ranges.append(textRange);
    return replaceText(charIndex, charCount, ranges);
}

bool ArtisticTextShape::replaceText(int charIndex, int charCount, const QList<ArtisticTextRange> &textRanges)
{
    CharIndex charPos = indexOfChar(charIndex);
    if (charPos.first < 0 || !charCount) {
        return false;
    }

    beginTextUpdate();

    removeText(charIndex, charCount);
    insertText(charIndex, textRanges);

    finishTextUpdate();

    return true;
}

qreal ArtisticTextShape::charAngleAt(int charIndex) const
{
    if (isOnPath()) {
        qreal t = m_charOffsets.value(qBound(0, charIndex, m_charOffsets.size() - 1));
        return m_baseline.angleAtPercent(t);
    }

    return 0.0;
}

QPointF ArtisticTextShape::charPositionAt(int charIndex) const
{
    return m_charPositions.value(qBound(0, charIndex, m_charPositions.size() - 1));
}

QRectF ArtisticTextShape::charExtentsAt(int charIndex) const
{
    CharIndex charPos = indexOfChar(charIndex);
    if (charIndex < 0 || isEmpty()) {
        charPos = CharIndex(0, 0);
    } else if (charPos.first < 0) {
        charPos = CharIndex(m_ranges.count() - 1, m_ranges.last().text().length() - 1);
    }

    if (charPos.first < m_ranges.size()) {
        const ArtisticTextRange &range = m_ranges.at(charPos.first);
        QFontMetrics metrics(range.font());
        int w = metrics.charWidth(range.text(), charPos.second);
        return QRectF(0, 0, w, metrics.height());
    }

    return QRectF();
}

void ArtisticTextShape::updateSizeAndPosition(bool global)
{
    QTransform shapeTransform = absoluteTransformation();

    // determine baseline position in document coordinates
    QPointF oldBaselinePosition = shapeTransform.map(QPointF(0, baselineOffset()));

    createOutline();

    QRectF bbox = m_outline.boundingRect();
    if (bbox.isEmpty()) {
        bbox = nullBoundBox();
    }

    if (isOnPath()) {
        // calculate the offset we have to apply to keep our position
        QPointF offset = m_outlineOrigin - bbox.topLeft();
        // cache topleft corner of baseline path
        m_outlineOrigin = bbox.topLeft();
        // the outline position is in document coordinates
        // so we adjust our position
        QTransform m;
        m.translate(-offset.x(), -offset.y());
        global ? applyAbsoluteTransformation(m) : applyTransformation(m);
    } else {
        // determine the new baseline position in document coordinates
        QPointF newBaselinePosition = shapeTransform.map(QPointF(0, -bbox.top()));
        // apply a transformation to compensate any translation of
        // our baseline position
        QPointF delta = oldBaselinePosition - newBaselinePosition;
        QTransform m;
        m.translate(delta.x(), delta.y());
        applyAbsoluteTransformation(m);
    }

    setSize(bbox.size());

    // map outline to shape coordinate system
    QTransform normalizeMatrix;
    normalizeMatrix.translate(-bbox.left(), -bbox.top());
    m_outline = normalizeMatrix.map(m_outline);
    const int charCount = m_charPositions.count();
    for (int i = 0; i < charCount; ++i) {
        m_charPositions[i] = normalizeMatrix.map(m_charPositions[i]);
    }
}

void ArtisticTextShape::cacheGlyphOutlines()
{
    m_charOutlines.clear();

    Q_FOREACH (const ArtisticTextRange &range, m_ranges) {
        const QString rangeText = range.text();
        const QFont rangeFont(range.font(), &m_paintDevice);
        const int textLength = rangeText.length();
        for (int charIdx = 0; charIdx < textLength; ++charIdx) {
            QPainterPath charOutline;
            charOutline.addText(QPointF(), rangeFont, rangeText[charIdx]);
            m_charOutlines.append(charOutline);
        }
    }
}

void ArtisticTextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    if (m_path && shape == m_path) {
        if (type == KoShape::Deleted) {
            // baseline shape was deleted
            m_path = 0;
        } else if (type == KoShape::ParentChanged && !shape->parent()) {
            // baseline shape was probably removed from the document
            m_path->removeDependee(this);
            m_path = 0;
        } else {
            update();
            // use the paths outline converted to document coordinates as the baseline
            m_baseline = m_path->absoluteTransformation().map(m_path->outline());
            updateSizeAndPosition(true);
            update();
        }
    }
}

CharIndex ArtisticTextShape::indexOfChar(int charIndex) const
{
    if (isEmpty()) {
        return CharIndex(-1, -1);
    }

    int rangeIndex = 0;
    int textLength = 0;
    Q_FOREACH (const ArtisticTextRange &range, m_ranges) {
        const int rangeTextLength = range.text().length();
        if (static_cast<int>(charIndex) < textLength + rangeTextLength) {
            return CharIndex(rangeIndex, charIndex - textLength);
        }
        textLength += rangeTextLength;
        rangeIndex++;
    }

    return CharIndex(-1, -1);
}

void ArtisticTextShape::beginTextUpdate()
{
    if (m_textUpdateCounter) {
        return;
    }

    m_textUpdateCounter++;
    update();
}

void ArtisticTextShape::finishTextUpdate()
{
    if (!m_textUpdateCounter) {
        return;
    }

    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();

    m_textUpdateCounter--;
}

bool ArtisticTextShape::saveSvg(SvgSavingContext &context)
{
    context.shapeWriter().startElement("text", false);
    context.shapeWriter().addAttribute("id", context.getID(this));

    SvgStyleWriter::saveSvgStyle(this, context);

    const QList<ArtisticTextRange> formattedText = text();

    // if we have only a single text range, save the font on the text element
    const bool hasSingleRange = formattedText.size() == 1;
    if (hasSingleRange) {
        saveSvgFont(formattedText.first().font(), context);
    }

    qreal anchorOffset = 0.0;
    if (textAnchor() == ArtisticTextShape::AnchorMiddle) {
        anchorOffset += 0.5 * this->size().width();
        context.shapeWriter().addAttribute("text-anchor", "middle");
    } else if (textAnchor() == ArtisticTextShape::AnchorEnd) {
        anchorOffset += this->size().width();
        context.shapeWriter().addAttribute("text-anchor", "end");
    }

    // check if we are set on a path
    if (layout() == ArtisticTextShape::Straight) {
        context.shapeWriter().addAttribute("x", anchorOffset);
        context.shapeWriter().addAttribute("y", baselineOffset());
        SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
        Q_FOREACH (const ArtisticTextRange &range, formattedText) {
            saveSvgTextRange(range, context, !hasSingleRange, baselineOffset());
        }
    } else {
        KoPathShape *baselineShape = KoPathShape::createShapeFromPainterPath(baseline());

        QString id = context.createUID("baseline");
        context.styleWriter().startElement("path");
        context.styleWriter().addAttribute("id", id);
        context.styleWriter().addAttribute("d", baselineShape->toString(baselineShape->absoluteTransformation() * context.userSpaceTransform()));
        context.styleWriter().endElement();

        context.shapeWriter().startElement("textPath");
        context.shapeWriter().addAttribute("xlink:href", QLatin1Char('#') + id);
        if (startOffset() > 0.0) {
            context.shapeWriter().addAttribute("startOffset", QString("%1%").arg(startOffset() * 100.0));
        }
        Q_FOREACH (const ArtisticTextRange &range, formattedText) {
            saveSvgTextRange(range, context, !hasSingleRange, baselineOffset());
        }
        context.shapeWriter().endElement();

        delete baselineShape;
    }

    context.shapeWriter().endElement();

    return true;
}

void ArtisticTextShape::saveSvgFont(const QFont &font, SvgSavingContext &context)
{
    context.shapeWriter().addAttribute("font-family", font.family());
    context.shapeWriter().addAttribute("font-size", font.pointSizeF());

    if (font.bold()) {
        context.shapeWriter().addAttribute("font-weight", "bold");
    }
    if (font.italic()) {
        context.shapeWriter().addAttribute("font-style", "italic");
    }
}

void ArtisticTextShape::saveSvgTextRange(const ArtisticTextRange &range, SvgSavingContext &context, bool saveRangeFont, qreal baselineOffset)
{
    context.shapeWriter().startElement("tspan", false);
    if (range.hasXOffsets()) {
        const char *attributeName = (range.xOffsetType() == ArtisticTextRange::AbsoluteOffset ? "x" : "dx");
        QString attributeValue;
        int charIndex = 0;
        while (range.hasXOffset(charIndex)) {
            if (charIndex) {
                attributeValue += QLatin1Char(',');
            }
            attributeValue += QString("%1").arg(SvgUtil::toUserSpace(range.xOffset(charIndex++)));
        }
        context.shapeWriter().addAttribute(attributeName, attributeValue);
    }
    if (range.hasYOffsets()) {
        if (range.yOffsetType() != ArtisticTextRange::AbsoluteOffset) {
            baselineOffset = 0;
        }
        const char *attributeName = (range.yOffsetType() == ArtisticTextRange::AbsoluteOffset ? " y" : " dy");
        QString attributeValue;
        int charIndex = 0;
        while (range.hasYOffset(charIndex)) {
            if (charIndex) {
                attributeValue += QLatin1Char(',');
            }
            attributeValue += QString("%1").arg(SvgUtil::toUserSpace(baselineOffset + range.yOffset(charIndex++)));
        }
        context.shapeWriter().addAttribute(attributeName, attributeValue);
    }
    if (range.hasRotations()) {
        QString attributeValue;
        int charIndex = 0;
        while (range.hasRotation(charIndex)) {
            if (charIndex) {
                attributeValue += ',';
            }
            attributeValue += QString("%1").arg(range.rotation(charIndex++));
        }
        context.shapeWriter().addAttribute("rotate", attributeValue);
    }
    if (range.baselineShift() != ArtisticTextRange::None) {
        switch (range.baselineShift()) {
        case ArtisticTextRange::Sub:
            context.shapeWriter().addAttribute("baseline-shift", "sub");
            break;
        case ArtisticTextRange::Super:
            context.shapeWriter().addAttribute("baseline-shift", "super");
            break;
        case ArtisticTextRange::Percent:
            context.shapeWriter().addAttribute("baseline-shift", QString("%1%").arg(range.baselineShiftValue() * 100));
            break;
        case ArtisticTextRange::Length:
            context.shapeWriter().addAttribute("baseline-shift", QString("%1%").arg(SvgUtil::toUserSpace(range.baselineShiftValue())));
            break;
        default:
            break;
        }
    }
    if (saveRangeFont) {
        saveSvgFont(range.font(), context);
    }
    context.shapeWriter().addTextNode(range.text());
    context.shapeWriter().endElement();
}

bool ArtisticTextShape::loadSvg(const KoXmlElement &textElement, SvgLoadingContext &context)
{
    clear();

    QString anchor;
    if (!textElement.attribute("text-anchor").isEmpty()) {
        anchor = textElement.attribute("text-anchor");
    }

    SvgStyles elementStyles = context.styleParser().collectStyles(textElement);
    context.styleParser().parseFont(elementStyles);

    ArtisticTextLoadingContext textContext;
    textContext.parseCharacterTransforms(textElement, context.currentGC());

    KoXmlElement parentElement = textElement;
    // first check if we have a "textPath" child element
    for (KoXmlNode n = textElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
        KoXmlElement e = n.toElement();
        if (e.tagName() == "textPath") {
            parentElement = e;
            break;
        }
    }

    KoPathShape *path = 0;
    bool pathInDocument = false;
    double offset = 0.0;

    const bool hasTextPathElement = parentElement != textElement && parentElement.hasAttribute("xlink:href");
    if (hasTextPathElement) {
        // create the referenced path shape
        context.pushGraphicsContext(parentElement);
        context.styleParser().parseFont(context.styleParser().collectStyles(parentElement));
        textContext.pushCharacterTransforms();
        textContext.parseCharacterTransforms(parentElement, context.currentGC());

        QString href = parentElement.attribute("xlink:href").mid(1);
        if (context.hasDefinition(href)) {
            const KoXmlElement &p = context.definition(href);
            // must be a path element as per svg spec
            if (p.tagName() == "path") {
                pathInDocument = false;
                path = new KoPathShape();
                path->clear();

                KoPathShapeLoader loader(path);
                loader.parseSvg(p.attribute("d"), true);
                path->setPosition(path->normalize());

                QPointF newPosition = QPointF(SvgUtil::fromUserSpace(path->position().x()),
                                              SvgUtil::fromUserSpace(path->position().y()));
                QSizeF newSize = QSizeF(SvgUtil::fromUserSpace(path->size().width()),
                                        SvgUtil::fromUserSpace(path->size().height()));

                path->setSize(newSize);
                path->setPosition(newPosition);
                path->applyAbsoluteTransformation(context.currentGC()->matrix);
            }
        } else {
            path = dynamic_cast<KoPathShape *>(context.shapeById(href));
            if (path) {
                pathInDocument = true;
            }
        }
        // parse the start offset
        if (! parentElement.attribute("startOffset").isEmpty()) {
            QString start = parentElement.attribute("startOffset");
            if (start.endsWith('%')) {
                offset = 0.01 * start.remove('%').toDouble();
            } else {
                const float pathLength = path ? path->outline().length() : 0.0;
                if (pathLength > 0.0) {
                    offset = start.toDouble() / pathLength;
                }
            }
        }
    }

    if (parentElement.hasChildNodes()) {
        // parse child elements
        parseTextRanges(parentElement, context, textContext);
        if (!context.currentGC()->preserveWhitespace) {
            const QString text = plainText();
            if (text.endsWith(' ')) {
                removeText(text.length() - 1, 1);
            }
        }
        setPosition(textContext.textPosition());
    } else {
        // a single text range
        appendText(createTextRange(textElement.text(), textContext, context.currentGC()));
        setPosition(textContext.textPosition());
    }

    if (hasTextPathElement) {
        if (path) {
            if (pathInDocument) {
                putOnPath(path);
            } else {
                putOnPath(path->absoluteTransformation().map(path->outline()));
                delete path;
            }

            if (offset > 0.0) {
                setStartOffset(offset);
            }
        }
        textContext.popCharacterTransforms();
        context.popGraphicsContext();
    }

    // adjust position by baseline offset
    if (! isOnPath()) {
        setPosition(position() - QPointF(0, baselineOffset()));
    }

    if (anchor == "middle") {
        setTextAnchor(ArtisticTextShape::AnchorMiddle);
    } else if (anchor == "end") {
        setTextAnchor(ArtisticTextShape::AnchorEnd);
    }

    return true;
}

void ArtisticTextShape::parseTextRanges(const KoXmlElement &element, SvgLoadingContext &context, ArtisticTextLoadingContext &textContext)
{
    for (KoXmlNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        KoXmlElement e = n.toElement();
        if (e.isNull()) {
            ArtisticTextRange range = createTextRange(n.toText().data(), textContext, context.currentGC());
            appendText(range);
        } else if (e.tagName() == "tspan") {
            SvgGraphicsContext *gc = context.pushGraphicsContext(e);
            context.styleParser().parseFont(context.styleParser().collectStyles(e));
            textContext.pushCharacterTransforms();
            textContext.parseCharacterTransforms(e, gc);
            parseTextRanges(e, context, textContext);
            textContext.popCharacterTransforms();
            context.popGraphicsContext();
        } else if (e.tagName() == "tref") {
            if (e.attribute("xlink:href").isEmpty()) {
                continue;
            }

            QString href = e.attribute("xlink:href").mid(1);
            ArtisticTextShape *refText = dynamic_cast<ArtisticTextShape *>(context.shapeById(href));
            if (refText) {
                foreach (const ArtisticTextRange &range, refText->text()) {
                    appendText(range);
                }
            } else if (context.hasDefinition(href)) {
                const KoXmlElement &p = context.definition(href);
                SvgGraphicsContext *gc = context.currentGC();
                appendText(ArtisticTextRange(textContext.simplifyText(p.text(), gc->preserveWhitespace), gc->font));
            }
        } else {
            continue;
        }
    }
}

ArtisticTextRange ArtisticTextShape::createTextRange(const QString &text, ArtisticTextLoadingContext &context, SvgGraphicsContext *gc)
{
    ArtisticTextRange range(context.simplifyText(text, gc->preserveWhitespace), gc->font);

    const int textLength = range.text().length();
    switch (context.xOffsetType()) {
    case ArtisticTextLoadingContext::Absolute:
        range.setXOffsets(context.xOffsets(textLength), ArtisticTextRange::AbsoluteOffset);
        break;
    case ArtisticTextLoadingContext::Relative:
        range.setXOffsets(context.xOffsets(textLength), ArtisticTextRange::RelativeOffset);
        break;
    default:
        // no x-offsets
        break;
    }
    switch (context.yOffsetType()) {
    case ArtisticTextLoadingContext::Absolute:
        range.setYOffsets(context.yOffsets(textLength), ArtisticTextRange::AbsoluteOffset);
        break;
    case ArtisticTextLoadingContext::Relative:
        range.setYOffsets(context.yOffsets(textLength), ArtisticTextRange::RelativeOffset);
        break;
    default:
        // no y-offsets
        break;
    }

    range.setRotations(context.rotations(textLength));

#if 0
    range.setLetterSpacing(gc->letterSpacing);
    range.setWordSpacing(gc->wordSpacing);

    if (gc->baselineShift == "sub") {
        range.setBaselineShift(ArtisticTextRange::Sub);
    } else if (gc->baselineShift == "super") {
        range.setBaselineShift(ArtisticTextRange::Super);
    } else if (gc->baselineShift.endsWith('%')) {
        range.setBaselineShift(ArtisticTextRange::Percent, SvgUtil::fromPercentage(gc->baselineShift));
    } else {
        qreal value = SvgUtil::parseUnitX(gc, gc->baselineShift);
        if (value != 0.0) {
            range.setBaselineShift(ArtisticTextRange::Length, value);
        }
    }
#endif

    //range.printDebug();

    return range;
}
