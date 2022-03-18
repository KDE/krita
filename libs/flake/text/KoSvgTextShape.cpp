/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"

#include <QTextLayout>

#include <raqm.h>
#include <fontconfig/fontconfig.h>
#include <klocalizedstring.h>

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include <KoShapeContainer_p.h>
#include <text/KoSvgTextChunkShape_p.h>
#include <text/KoSvgTextShapeMarkupConverter.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>

#include "kis_debug.h"

#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
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
#include <QPainterPath>
#include <QFileInfo>
#include <boost/optional.hpp>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>

#include <FlakeDebug.h>

#include <QSharedData>


class KoSvgTextShape::Private
{
public:

    // NOTE: the cache data is shared between all the instances of
    //       the shape, though it will be reset locally if the
    //       accessing thread changes
    std::vector<raqm_t*> cachedLayouts;
    std::vector<QPointF> cachedLayoutsOffsets;
    QThread *cachedLayoutsWorkingThread = 0;
    FT_Library library = NULL;

    QPainterPath path;
    // temp
    QString fontName;


    void clearAssociatedOutlines(const KoShape *rootShape);

};

KoSvgTextShape::KoSvgTextShape()
    : KoSvgTextChunkShape()
    , d(new Private)
{
    setShapeId(KoSvgTextShape_SHAPEID);
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoSvgTextChunkShape(rhs)
    , d(new Private)
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

void KoSvgTextShape::paintComponent(QPainter &painter, KoShapePaintingContext &paintContext) const
{

    Q_UNUSED(paintContext);

    /**
     * HACK ALERT:
     * QTextLayout should only be accessed from the thread it has been created in.
     * If the cached layout has been created in a different thread, we should just
     * recreate the layouts in the current thread to be able to render them.
     */
    qDebug() << "paint component code";
    if (QThread::currentThread() != d->cachedLayoutsWorkingThread) {
        relayout();
    }

    /*for (int i = 0; i < (int)d->cachedLayouts.size(); i++) {
        //d->cachedLayouts[i]->draw(&painter, d->cachedLayoutsOffsets[i]);
    }*/
    qDebug() << "drawing...";
    painter.setBrush(Qt::black);
    painter.setOpacity(1.0);
    painter.drawPath(d->path);

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

void KoSvgTextShape::paintStroke(QPainter &painter, KoShapePaintingContext &paintContext) const
{
    Q_UNUSED(painter);
    Q_UNUSED(paintContext);

    // do nothing! everything is painted in paintComponent()
}

QPainterPath KoSvgTextShape::textOutline() const
{

    QPainterPath result;
    result.setFillRule(Qt::WindingFill);
    qDebug() << "starting creation textoutlne";

    for (int i = 0; i < (int)d->cachedLayouts.size(); i++) {
        qDebug() << "drawing layout" << i;
        //const QPointF layoutOffset = d->cachedLayoutsOffsets[i];
        //const QTextLayout *layout = d->cachedLayouts[i].get();
        size_t count = 0;
        raqm_glyph_t *glyphs = raqm_get_glyphs (d->cachedLayouts.at(i), &count);

        QPointF oldOffset = QPointF(0,0);
        const qreal factor = 1/64.;
        //QMap<QString, QRawFont> fontList;

        for (int j = 0; j < int(count); j++) {

            QPainterPath glyph;
            glyph.setFillRule(Qt::WindingFill);
            FT_Load_Glyph(glyphs[j].ftface, glyphs[j].index, 0);
            FT_Outline outline = glyphs[j].ftface->glyph->outline;

            int contour = 0;
            int lastContourPoint = outline.n_points - 1;
            if (outline.n_contours>0) {
                lastContourPoint = outline.contours[contour];
            }

            // Code taken from qfontengine_ft.cpp
            // TODO: Improve the transforms here, the coordinate system for a glyph is different
            // from that of a QPainterPath, it's origin at the (writing mode independant)
            // baseline instead of the topleft.

            QPointF cp = QPointF();
            // convert the outline to a painter path
            int i = 0;
            for (int j = 0; j < outline.n_contours; ++j) {
                int last_point = outline.contours[j];
                //qDebug() << "contour:" << i << "to" << last_point;
                QPointF start = QPointF(outline.points[i].x*factor, -outline.points[i].y*factor);
                if (!(outline.tags[i] & 1)) {               // start point is not on curve:
                    if (!(outline.tags[last_point] & 1)) {  // end point is not on curve:
                        //qDebug() << "  start and end point are not on curve";
                        start = (QPointF(outline.points[last_point].x*factor,
                                         -outline.points[last_point].y*factor) + start) / 2.0;
                    } else {
                        //qDebug() << "  end point is on curve, start is not";
                        start = QPointF(outline.points[last_point].x*factor,
                                        -outline.points[last_point].y*factor);
                    }
                    --i;   // to use original start point as control point below
                }
                start += cp;
                //qDebug() << "  start at" << start;
                glyph.moveTo(start);
                QPointF c[4];
                c[0] = start;
                int n = 1;
                while (i < last_point) {
                    ++i;
                    c[n] = cp + QPointF(outline.points[i].x*factor, -outline.points[i].y*factor);
                    //qDebug() << "    " << i << c[n] << "tag =" << (int)g->outline.tags[i]
                    //                   << ": on curve =" << (bool)(g->outline.tags[i] & 1);
                    ++n;
                    switch (outline.tags[i] & 3) {
                    case 2:
                        // cubic bezier element
                        if (n < 4)
                            continue;
                        c[3] = (c[3] + c[2])/2;
                        --i;
                        break;
                    case 0:
                        // quadratic bezier element
                        if (n < 3)
                            continue;
                        c[3] = (c[1] + c[2])/2;
                        c[2] = (2*c[1] + c[3])/3;
                        c[1] = (2*c[1] + c[0])/3;
                        --i;
                        break;
                    case 1:
                    case 3:
                        if (n == 2) {
                            //qDebug() << "  lineTo" << c[1];
                            glyph.lineTo(c[1]);
                            c[0] = c[1];
                            n = 1;
                            continue;
                        } else if (n == 3) {
                            c[3] = c[2];
                            c[2] = (2*c[1] + c[3])/3;
                            c[1] = (2*c[1] + c[0])/3;
                        }
                        break;
                    }
                    //qDebug() << "  cubicTo" << c[1] << c[2] << c[3];
                    glyph.cubicTo(c[1], c[2], c[3]);
                    c[0] = c[3];
                    n = 1;
                }
                if (n == 1) {
                    //qDebug() << "  closeSubpath";
                    glyph.closeSubpath();
                } else {
                    c[3] = start;
                    if (n == 2) {
                        c[2] = (2*c[1] + c[3])/3;
                        c[1] = (2*c[1] + c[0])/3;
                    }
                    //qDebug() << "  close cubicTo" << c[1] << c[2] << c[3];
                    glyph.cubicTo(c[1], c[2], c[3]);
                }
                ++i;
            }
            QPointF offset = QPointF(glyphs[j].x_offset * factor, glyphs[j].y_offset * factor);
            QPointF advance = QPointF(glyphs[j].x_advance * factor, glyphs[j].y_advance * factor);
            qDebug() << "adding glyph" << j << "offset" << offset << "advance" << advance;
            qDebug() << "boundingRect" << glyph.boundingRect();
            // According to the harfbuzz docs, the offset doesn't advance the line, just offsets.
            glyph.translate(oldOffset + offset);
            result.addPath(glyph);
            oldOffset += advance;
        }

        /*
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
        }*/
    }
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(this);
    while (chunkShape->shapes().size() > 0 && chunkShape->isTextNode() != true){
        chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(chunkShape->shapes().first());
    }
    chunkShape->layoutInterface()->addAssociatedOutline(result.boundingRect());

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

void KoSvgTextShape::relayout() const
{
    d->cachedLayouts.clear();
    d->cachedLayoutsOffsets.clear();
    d->cachedLayoutsWorkingThread = QThread::currentThread();
    d->path.clear();

    QPointF currentTextPos;

    QVector<TextChunk> textChunks = mergeIntoChunks(layoutInterface()->collectSubChunks());

    Q_FOREACH (const TextChunk &chunk, textChunks) {
        raqm_t *layout(raqm_create());

        // QTextOption option;

        // WARNING: never activate this option! It breaks the RTL text layout!
        //option.setFlags(QTextOption::ShowTabsAndSpaces);

        //option.setWrapMode(QTextOption::WrapAnywhere);
        //option.setUseDesignMetrics(true); // TODO: investigate if it is needed?
        //option.setTextDirection(chunk.direction);

        if (!d->library) {
            FT_Init_FreeType(&d->library);
        }
        FcConfig *config = FcConfigGetCurrent();
        FcObjectSet *objectSet = FcObjectSetBuild(FC_FAMILY,FC_FILE, nullptr);

        QMap<QString,FT_Face> faces;

        if (raqm_set_text_utf8(layout, chunk.text.toUtf8(), chunk.text.size())) {
            if (chunk.direction == Qt::RightToLeft) {
                raqm_set_par_direction(layout, raqm_direction_t::RAQM_DIRECTION_RTL);
            } else {
                raqm_set_par_direction(layout, raqm_direction_t::RAQM_DIRECTION_LTR);
            }

            for (QTextLayout::FormatRange format : chunk.formats) {
                FT_Face face = NULL;

                FcPattern *p = FcPatternCreate();
                const FcChar8 *vals = reinterpret_cast<FcChar8*>(format.format.font().family().toUtf8().data());
                qreal fontSize = format.format.font().pointSizeF();
                FcPatternAddString(p, FC_FAMILY, vals);
                FcFontSet *fontSet = FcFontList(config, p, objectSet);
                QString fontFileName;
                QStringList fontProperties = QString(reinterpret_cast<char*>(FcNameUnparse(fontSet->fonts[0]))).split(':');
                for (QString value : fontProperties) {
                    if (value.startsWith("file")) {
                        fontFileName = value.split("=").last();
                        fontFileName.remove("\\");
                    }
                }

                /*if (faces.contains(fontFileName)) {
                    qDebug() << "found face" << fontFileName;
                    face = faces.value(fontFileName);
                    raqm_set_freetype_face_range(layout, face, format.start, format.length);
                } else*/ if (FT_New_Face(d->library, fontFileName.toUtf8().data(), 0, &face) == 0) {
                    qDebug() << "face loaded" << fontFileName;
                    FT_Set_Char_Size(face, fontSize*64.0, 0, 0, 0);
                    if (format.start == 0) {
                        raqm_set_freetype_face(layout, face);
                    }
                    raqm_set_freetype_face_range(layout, face, format.start, format.length);
                    //faces.insert(fontFileName, face);
                } else {
                    qDebug() << "face did not load" << fontFileName;
                }
            }
        }

        if (raqm_layout(layout)) {
            qDebug() << "layout succeeded";
        }

        currentTextPos = chunk.applyStartPosOverride(currentTextPos);
        const QPointF anchorPointPos = currentTextPos;

        /*
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
        */

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
    // Calculate the associated outline for a text chunk.
    d->clearAssociatedOutlines(this);

    if (d->path.isEmpty()) {
        d->path = textOutline();
    }
    /*
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
                     * BUG: 420408 (vertically)
                     *
                    rect.setLeft(qMin(rect.left(), lastGlyphRect.left()) - 0.5 * firstGlyphRect.width());
                    rect.setRight(qMax(rect.right(), lastGlyphRect.right()) + 0.5 * lastGlyphRect.width());

                    rect.adjust(0, -0.5*rect.height(), 0, 0.5*rect.height()); // add some vertical margin too

                    wrapper.addCharacterRect(rect.translated(layoutOffset));
                }
            }
        }
    }
    */
}

void KoSvgTextShape::Private::clearAssociatedOutlines(const KoShape *rootShape)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(rootShape);
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
    : KoShapeFactoryBase(KoSvgTextShape_SHAPEID, i18nc("Text label in SVG Text Tool", "Text"))
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
    converter.convertFromSvg(i18nc("Default text for the text shape", "<text>Placeholder Text</text>"),
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

    QString svgText = params->stringProperty("svgText", i18nc("Default text for the text shape", "<text>Placeholder Text</text>"));
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

bool KoSvgTextShapeFactory::supports(const QDomElement &/*e*/, KoShapeLoadingContext &/*context*/) const
{
    return false;
}
