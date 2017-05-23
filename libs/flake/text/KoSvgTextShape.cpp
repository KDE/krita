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

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include <KoShapeContainer_p.h>
#include <text/KoSvgTextChunkShape_p.h>

#include "kis_debug.h"
#include <KoXmlReader.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>

#include <vector>
#include <memory>
#include <QPainter>
#include <boost/optional.hpp>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>


struct KoSvgTextShapePrivate : public KoSvgTextChunkShapePrivate
{
    KoSvgTextShapePrivate(KoSvgTextShape *_q)
        : KoSvgTextChunkShapePrivate(_q)
    {
    }

    KoSvgTextShapePrivate(const KoSvgTextShapePrivate &rhs, KoSvgTextShape *q)
        : KoSvgTextChunkShapePrivate(rhs, q)
    {
    }

    std::vector<std::unique_ptr<QTextLayout>> layouts;
    std::vector<QPointF> layoutOffsets;


    Q_DECLARE_PUBLIC(KoSvgTextShape)
};

KoSvgTextShape::KoSvgTextShape()
    : KoSvgTextChunkShape(new KoSvgTextShapePrivate(this))
{
    Q_D(KoSvgTextShape);
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoSvgTextChunkShape(new KoSvgTextShapePrivate(*rhs.d_func(), this))
{
    Q_D(KoSvgTextShape);
}

KoSvgTextShape::~KoSvgTextShape()
{
}

KoShape *KoSvgTextShape::cloneShape() const
{
    return new KoSvgTextShape(*this);
}

void KoSvgTextShape::paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    Q_D(KoSvgTextShape);

    Q_UNUSED(paintContext);

    applyConversion(painter, converter);
    for (int i = 0; i < (int)d->layouts.size(); i++) {
        d->layouts[i]->draw(&painter, d->layoutOffsets[i]);
    }
}

struct TextChunk {
    QString text;
    QList<QTextLayout::FormatRange> formats;
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
            o.offset = it->transformation.adjustRelativePos();

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


void KoSvgTextShape::relayout()
{
    Q_D(KoSvgTextShape);

    QPointF currentTextPos;

    QVector<TextChunk> textChunks = mergeIntoChunks(layoutInterface()->collectSubChunks());

    Q_FOREACH (const TextChunk &chunk, textChunks) {
        std::unique_ptr<QTextLayout> layout(new QTextLayout());

        QTextOption option;

        // WARNING: never activate this option! It breaks the RTL text layout!
        //option.setFlags(QTextOption::ShowTabsAndSpaces);

        option.setWrapMode(QTextOption::WrapAnywhere);
        option.setUseDesignMetrics(true); // TODO: investigate if it is needed?
        option.setTextDirection(chunk.direction);

        layout->setText(chunk.text);
        layout->setTextOption(option);
        layout->setAdditionalFormats(chunk.formats);
        layout->setCacheEnabled(true);

        layout->beginLayout();

        currentTextPos = chunk.applyStartPosOverride(currentTextPos);
        const QPointF anchorPointPos = currentTextPos;

        int lastSubChunkStart = 0;
        QPointF lastSubChunkOffset;

        for (int i = 0; i <= chunk.offsets.size(); i++) {
            if (i == chunk.offsets.size()) {
                const int numChars = chunk.text.size() - lastSubChunkStart;
                if (!numChars) break;

                // fix trailing whitespace problem
                if (numChars == 1 && chunk.text[i] == ' ') {
                    QFontMetrics metrics(chunk.formats.last().format.font());
                    currentTextPos.rx() += metrics.width(' ');
                    break;
                }

                QTextLine line = layout->createLine();
                KIS_SAFE_ASSERT_RECOVER(line.isValid()) { break; }

                currentTextPos += lastSubChunkOffset;

                line.setNumColumns(numChars);
                line.setPosition(currentTextPos);
                currentTextPos.rx() += line.horizontalAdvance();


            } else {
                const int numChars = chunk.offsets[i].start - lastSubChunkStart;

                if (numChars > 0) {
                    QTextLine line = layout->createLine();
                    KIS_SAFE_ASSERT_RECOVER(line.isValid()) { break; }
                    line.setNumColumns(numChars);

                    currentTextPos += lastSubChunkOffset;
                    line.setPosition(currentTextPos);
                    currentTextPos.rx() += line.horizontalAdvance();
                }

                lastSubChunkStart = chunk.offsets[i].start;
                lastSubChunkOffset = chunk.offsets[i].offset;
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

        d->layouts.push_back(std::move(layout));
        d->layoutOffsets.push_back(-diff);

    }
}




























