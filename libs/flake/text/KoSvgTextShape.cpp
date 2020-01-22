/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoSvgTextShape.h"

#include <QTextLayout>
#include <klocalizedstring.h>

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include <KoShapeContainer_p.h>
#include <text/KoSvgTextChunkShape_p.h>
#include <text/KoSvgTextShapeMarkupConverter.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>

#include "kis_debug.h"

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoIcon.h>
#include <KoProperties.h>
#include <KoColorBackground.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>

#include <QApplication>
#include <QThread>
#include <vector>
#include <memory>
#include <QPainter>
#include <boost/optional.hpp>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>

#include <FlakeDebug.h>

#include <QSharedData>

class KoSvgTextShape::Private : public QSharedData
{
public:

    // NOTE: the cache data is shared between all the instances of
    //       the shape, though it will be reset locally if the
    //       accessing thread changes
    std::vector<std::shared_ptr<QTextLayout>> cachedLayouts;
    std::vector<QPointF> cachedLayoutsOffsets;
    QThread *cachedLayoutsWorkingThread = 0;


    void clearAssociatedOutlines(KoShape *rootShape);

};

KoSvgTextShape::KoSvgTextShape()
    : KoSvgTextChunkShape()
    , d(new Private)
{
    setShapeId(KoSvgTextShape_SHAPEID);
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoSvgTextChunkShape(rhs)
    , d(rhs.d)
{
    setShapeId(KoSvgTextShape_SHAPEID);
    // QTextLayout has no copy-ctor, so just relayout everything!
    relayout();
}

KoSvgTextShape::~KoSvgTextShape()
{
}

KoShape *KoSvgTextShape::cloneShape() const
{
    return new KoSvgTextShape(*this);
}

void KoSvgTextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    KoSvgTextChunkShape::shapeChanged(type, shape);

    if (type == StrokeChanged || type == BackgroundChanged || type == ContentChanged) {
        relayout();
    }
}

void KoSvgTextShape::paintComponent(QPainter &painter, KoShapePaintingContext &paintContext)
{

    Q_UNUSED(paintContext);

    /**
     * HACK ALERT:
     * QTextLayout should only be accessed from the thread it has been created in.
     * If the cached layout has been created in a different thread, we should just
     * recreate the layouts in the current thread to be able to render them.
     */

    if (QThread::currentThread() != d->cachedLayoutsWorkingThread) {
        relayout();
    }

    for (int i = 0; i < (int)d->cachedLayouts.size(); i++) {
        d->cachedLayouts[i]->draw(&painter, d->cachedLayoutsOffsets[i]);
    }

    /**
     * HACK ALERT:
     * The layouts of non-gui threads must be destroyed in the same thread
     * they have been created. Because the thread might be restarted in the
     * meantime or just destroyed, meaning that the per-thread freetype data
     * will not be available.
     */
    if (QThread::currentThread() != qApp->thread()) {
        d->cachedLayouts.clear();
        d->cachedLayoutsOffsets.clear();
        d->cachedLayoutsWorkingThread = 0;
    }
}

void KoSvgTextShape::paintStroke(QPainter &painter, KoShapePaintingContext &paintContext)
{
    Q_UNUSED(painter);
    Q_UNUSED(paintContext);

    // do nothing! everything is painted in paintComponent()
}

QPainterPath KoSvgTextShape::textOutline()
{

    QPainterPath result;
    result.setFillRule(Qt::WindingFill);


    for (int i = 0; i < (int)d->cachedLayouts.size(); i++) {
        const QPointF layoutOffset = d->cachedLayoutsOffsets[i];
        const QTextLayout *layout = d->cachedLayouts[i].get();

        for (int j = 0; j < layout->lineCount(); j++) {
            QTextLine line = layout->lineAt(j);

            Q_FOREACH (const QGlyphRun &run, line.glyphRuns()) {
                const QVector<quint32> indexes = run.glyphIndexes();
                const QVector<QPointF> positions = run.positions();
                const QRawFont font = run.rawFont();

                KIS_SAFE_ASSERT_RECOVER(indexes.size() == positions.size()) { continue; }

                for (int k = 0; k < indexes.size(); k++) {
                    QPainterPath glyph = font.pathForGlyph(indexes[k]);
                    glyph.translate(positions[k] + layoutOffset);
                    result += glyph;
                }

                const qreal thickness = font.lineThickness();
                const QRectF runBounds = run.boundingRect();

                if (run.overline()) {
                    // the offset is calculated to be consistent with the way how Qt renders the text
                    const qreal y = line.y();
                    QRectF overlineBlob(runBounds.x(), y, runBounds.width(), thickness);
                    overlineBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(overlineBlob);

                    // don't use direct addRect, because it doesn't care about Qt::WindingFill
                    result += path;
                }

                if (run.strikeOut()) {
                    // the offset is calculated to be consistent with the way how Qt renders the text
                    const qreal y = line.y() + 0.5 * line.height();
                    QRectF strikeThroughBlob(runBounds.x(), y, runBounds.width(), thickness);
                    strikeThroughBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(strikeThroughBlob);

                    // don't use direct addRect, because it doesn't care about Qt::WindingFill
                    result += path;
                }

                if (run.underline()) {
                    const qreal y = line.y() + line.ascent() + font.underlinePosition();
                    QRectF underlineBlob(runBounds.x(), y, runBounds.width(), thickness);
                    underlineBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(underlineBlob);

                    // don't use direct addRect, because it doesn't care about Qt::WindingFill
                    result += path;
                }
            }
        }
    }

    return result;
}

void KoSvgTextShape::resetTextShape()
{
    KoSvgTextChunkShape::resetTextShape();
    relayout();
}

struct TextChunk {
    QString text;
    QVector<QTextLayout::FormatRange> formats;
    Qt::LayoutDirection direction = Qt::LeftToRight;
    Qt::Alignment alignment = Qt::AlignLeading;

    struct SubChunkOffset {
        QPointF offset;
        int start = 0;
    };

    QVector<SubChunkOffset> offsets;

    boost::optional<qreal> xStartPos;
    boost::optional<qreal> yStartPos;

    QPointF applyStartPosOverride(const QPointF &pos) const {
        QPointF result = pos;

        if (xStartPos) {
            result.rx() = *xStartPos;
        }

        if (yStartPos) {
            result.ry() = *yStartPos;
        }

        return result;
    }
};

QVector<TextChunk> mergeIntoChunks(const QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> &subChunks)
{
    QVector<TextChunk> chunks;

    for (auto it = subChunks.begin(); it != subChunks.end(); ++it) {
        if (it->transformation.startsNewChunk() || it == subChunks.begin()) {
            TextChunk newChunk = TextChunk();
            newChunk.direction = it->format.layoutDirection();
            newChunk.alignment = it->format.calculateAlignment();
            newChunk.xStartPos = it->transformation.xPos;
            newChunk.yStartPos = it->transformation.yPos;
            chunks.append(newChunk);
        }

        TextChunk &currentChunk = chunks.last();

        if (it->transformation.hasRelativeOffset()) {
            TextChunk::SubChunkOffset o;
            o.start = currentChunk.text.size();
            o.offset = it->transformation.relativeOffset();

            KIS_SAFE_ASSERT_RECOVER_NOOP(!o.offset.isNull());
            currentChunk.offsets.append(o);
        }

        QTextLayout::FormatRange formatRange;
        formatRange.start = currentChunk.text.size();
        formatRange.length = it->text.size();
        formatRange.format = it->format;

        currentChunk.formats.append(formatRange);

        currentChunk.text += it->text;
    }

    return chunks;
}

/**
 * Qt's QTextLayout has a weird trait, it doesn't count space characters as
 * distinct characters in QTextLayout::setNumColumns(), that is, if we want to
 * position a block of text that starts with a space character in a specific
 * position, QTextLayout will drop this space and will move the text to the left.
 *
 * That is why we have a special wrapper object that ensures that no spaces are
 * dropped and their horizontal advance parameter is taken into account.
 */
struct LayoutChunkWrapper
{
    LayoutChunkWrapper(QTextLayout *layout)
        : m_layout(layout)
    {
    }

    QPointF addTextChunk(int startPos, int length, const QPointF &textChunkStartPos)
    {
        QPointF currentTextPos = textChunkStartPos;

        const int lastPos = startPos + length - 1;

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(startPos == m_addedChars, currentTextPos);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(lastPos < m_layout->text().size(), currentTextPos);

        //        qDebug() << m_layout->text();

        QTextLine line;
        std::swap(line, m_danglingLine);

        if (!line.isValid()) {
            line = m_layout->createLine();
        }

        // skip all the space characters that were not included into the Qt's text line
        const int currentLineStart = line.isValid() ? line.textStart() : startPos + length;
        while (startPos < currentLineStart && startPos <= lastPos) {
            currentTextPos.rx() += skipSpaceCharacter(startPos);
            startPos++;
        }

        if (startPos <= lastPos) {
            // defines the number of columns to look for glyphs
            const int numChars = lastPos - startPos + 1;
            // Tabs break the normal column flow
            // grow to avoid missing glyphs

            int charOffset = 0;
            int noChangeCount = 0;
            while (line.textLength() < numChars) {
                int tl = line.textLength();
                line.setNumColumns(numChars + charOffset);
                if (tl == line.textLength()) {
                    noChangeCount++;
                    // 5 columns max are needed to discover tab char. Set to 10 to be safe.
                    if (noChangeCount > 10) break;
                } else {
                    noChangeCount = 0;
                }
                charOffset++;
            }

            line.setPosition(currentTextPos - QPointF(0, line.ascent()));
            currentTextPos.rx() += line.horizontalAdvance();

            // skip all the space characters that were not included into the Qt's text line
            for (int i = line.textStart() + line.textLength(); i < lastPos; i++) {
                currentTextPos.rx() += skipSpaceCharacter(i);
            }

        } else {
            // keep the created but unused line for future use
            std::swap(line, m_danglingLine);
        }
        m_addedChars += length;

        return currentTextPos;
    }

private:
    qreal skipSpaceCharacter(int pos) {
        const QTextCharFormat format =
                formatForPos(pos, m_layout->formats());

        const QChar skippedChar = m_layout->text()[pos];
        KIS_SAFE_ASSERT_RECOVER_NOOP(skippedChar.isSpace() || !skippedChar.isPrint());

        QFontMetrics metrics(format.font());
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
        return metrics.horizontalAdvance(skippedChar);
#else
        return metrics.width(skippedChar);
#endif
    }

    static QTextCharFormat formatForPos(int pos, const QVector<QTextLayout::FormatRange> &formats)
    {
        Q_FOREACH (const QTextLayout::FormatRange &range, formats) {
            if (pos >= range.start && pos < range.start + range.length) {
                return range.format;
            }
        }

        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "pos should be within the bounds of the layouted text");

        return QTextCharFormat();
    }

private:
    int m_addedChars = 0;
    QTextLayout *m_layout;
    QTextLine m_danglingLine;
};

void KoSvgTextShape::relayout()
{

    d->cachedLayouts.clear();
    d->cachedLayoutsOffsets.clear();
    d->cachedLayoutsWorkingThread = QThread::currentThread();

    QPointF currentTextPos;

    QVector<TextChunk> textChunks = mergeIntoChunks(layoutInterface()->collectSubChunks());

    Q_FOREACH (const TextChunk &chunk, textChunks) {
        std::shared_ptr<QTextLayout> layout(new QTextLayout());

        QTextOption option;

        // WARNING: never activate this option! It breaks the RTL text layout!
        //option.setFlags(QTextOption::ShowTabsAndSpaces);

        option.setWrapMode(QTextOption::WrapAnywhere);
        option.setUseDesignMetrics(true); // TODO: investigate if it is needed?
        option.setTextDirection(chunk.direction);

        layout->setText(chunk.text);
        layout->setTextOption(option);
        layout->setFormats(chunk.formats);
        layout->setCacheEnabled(true);

        layout->beginLayout();

        currentTextPos = chunk.applyStartPosOverride(currentTextPos);
        const QPointF anchorPointPos = currentTextPos;

        int lastSubChunkStart = 0;
        QPointF lastSubChunkOffset;

        LayoutChunkWrapper wrapper(layout.get());

        for (int i = 0; i <= chunk.offsets.size(); i++) {
            const bool isFinalPass = i == chunk.offsets.size();

            const int length =
                    !isFinalPass ?
                        chunk.offsets[i].start - lastSubChunkStart :
                        chunk.text.size() - lastSubChunkStart;

            if (length > 0) {
                currentTextPos += lastSubChunkOffset;
                currentTextPos = wrapper.addTextChunk(lastSubChunkStart,
                                                      length,
                                                      currentTextPos);
            }

            if (!isFinalPass) {
                lastSubChunkOffset = chunk.offsets[i].offset;
                lastSubChunkStart = chunk.offsets[i].start;
            }
        }

        layout->endLayout();

        QPointF diff;

        if (chunk.alignment & Qt::AlignTrailing || chunk.alignment & Qt::AlignHCenter) {
            if (chunk.alignment & Qt::AlignTrailing) {
                diff = currentTextPos - anchorPointPos;
            } else if (chunk.alignment & Qt::AlignHCenter) {
                diff = 0.5 * (currentTextPos - anchorPointPos);
            }

            // TODO: fix after t2b text implemented
            diff.ry() = 0;
        }

        d->cachedLayouts.push_back(layout);
        d->cachedLayoutsOffsets.push_back(-diff);

    }

    d->clearAssociatedOutlines(this);

    for (int i = 0; i < int(d->cachedLayouts.size()); i++) {
        const QTextLayout &layout = *d->cachedLayouts[i];
        const QPointF layoutOffset = d->cachedLayoutsOffsets[i];

        using namespace KoSvgText;

        Q_FOREACH (const QTextLayout::FormatRange &range, layout.formats()) {
            const KoSvgCharChunkFormat &format =
                    static_cast<const KoSvgCharChunkFormat&>(range.format);
            AssociatedShapeWrapper wrapper = format.associatedShapeWrapper();

            const int rangeStart = range.start;
            const int safeRangeLength = range.length > 0 ? range.length : layout.text().size() - rangeStart;

            if (safeRangeLength <= 0) continue;

            const int rangeEnd = range.start + safeRangeLength - 1;

            const int firstLineIndex = layout.lineForTextPosition(rangeStart).lineNumber();
            const int lastLineIndex = layout.lineForTextPosition(rangeEnd).lineNumber();

            for (int i = firstLineIndex; i <= lastLineIndex; i++) {
                const QTextLine line = layout.lineAt(i);

                // It might happen that the range contains only one (or two)
                // symbol that is a whitespace symbol. In such a case we should
                // just skip this (invalid) line.
                if (!line.isValid()) continue;

                const int posStart = qMax(line.textStart(), rangeStart);
                const int posEnd = qMin(line.textStart() + line.textLength() - 1, rangeEnd);

                const QList<QGlyphRun> glyphRuns = line.glyphRuns(posStart, posEnd - posStart + 1);
                Q_FOREACH (const QGlyphRun &run, glyphRuns) {
                    const QPointF firstPosition = run.positions().first();
                    const quint32 firstGlyphIndex = run.glyphIndexes().first();

                    const QPointF lastPosition = run.positions().last();
                    const quint32 lastGlyphIndex = run.glyphIndexes().last();

                    const QRawFont rawFont = run.rawFont();

                    const QRectF firstGlyphRect = rawFont.boundingRect(firstGlyphIndex).translated(firstPosition);
                    const QRectF lastGlyphRect = rawFont.boundingRect(lastGlyphIndex).translated(lastPosition);

                    QRectF rect = run.boundingRect();

                    /**
                     * HACK ALERT: there is a bug in a way how Qt calculates boundingRect()
                     *             of the glyph run. It doesn't care about left and right bearings
                     *             of the border chars in the run, therefore it becomes cropped.
                     *
                     *             Here we just add a half-char width margin to both sides
                     *             of the glyph run to make sure the glyphs are fully painted.
                     *
                     * BUG: 389528
                     * BUG: 392068
                     */
                    rect.setLeft(qMin(rect.left(), lastGlyphRect.left()) - 0.5 * firstGlyphRect.width());
                    rect.setRight(qMax(rect.right(), lastGlyphRect.right()) + 0.5 * lastGlyphRect.width());

                    wrapper.addCharacterRect(rect.translated(layoutOffset));
                }
            }
        }
    }
}

void KoSvgTextShape::Private::clearAssociatedOutlines(KoShape *rootShape)
{
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    chunkShape->layoutInterface()->clearAssociatedOutline();

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        clearAssociatedOutlines(child);
    }
}

bool KoSvgTextShape::isRootTextNode() const
{
    return true;
}

KoSvgTextShapeFactory::KoSvgTextShapeFactory()
    : KoShapeFactoryBase(KoSvgTextShape_SHAPEID, i18n("Text"))
{
    setToolTip(i18n("SVG Text Shape"));
    setIconName(koIconNameCStr("x-shape-text"));
    setLoadingPriority(5);
    setXmlElementNames(KoXmlNS::svg, QStringList("text"));

    KoShapeTemplate t;
    t.name = i18n("SVG Text");
    t.iconName = koIconName("x-shape-text");
    t.toolTip = i18n("SVG Text Shape");
    addTemplate(t);
}

KoShape *KoSvgTextShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    debugFlake << "Create default svg text shape";

    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg("<text>Lorem ipsum dolor sit amet, consectetur adipiscing elit.</text>",
                             "<defs/>",
                             QRectF(0, 0, 200, 60),
                             documentResources->documentResolution());

    debugFlake << converter.errors() << converter.warnings();

    return shape;
}

KoShape *KoSvgTextShapeFactory::createShape(const KoProperties *params, KoDocumentResourceManager *documentResources) const
{
    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);

    QString svgText = params->stringProperty("svgText", "<text>Lorem ipsum dolor sit amet, consectetur adipiscing elit.</text>");
    QString defs = params->stringProperty("defs" , "<defs/>");
    QRectF shapeRect = QRectF(0, 0, 200, 60);
    QVariant rect = params->property("shapeRect");

    if (rect.type()==QVariant::RectF) {
        shapeRect = rect.toRectF();
    }

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg(svgText,
                             defs,
                             shapeRect,
                             documentResources->documentResolution());

    shape->setPosition(shapeRect.topLeft());

    return shape;
}

bool KoSvgTextShapeFactory::supports(const KoXmlElement &/*e*/, KoShapeLoadingContext &/*context*/) const
{
    return false;
}
