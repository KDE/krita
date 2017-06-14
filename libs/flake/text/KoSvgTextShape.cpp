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

#include "kis_debug.h"

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoIcon.h>

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

    void clearAssociatedOutlines(KoShape *rootShape);


    Q_DECLARE_PUBLIC(KoSvgTextShape)
};

KoSvgTextShape::KoSvgTextShape()
    : KoSvgTextChunkShape(new KoSvgTextShapePrivate(this))
{
    Q_D(KoSvgTextShape);
    setShapeId(KoSvgTextShape_SHAPEID);
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoSvgTextChunkShape(new KoSvgTextShapePrivate(*rhs.d_func(), this))
{
    Q_D(KoSvgTextShape);
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

void KoSvgTextShape::paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    Q_D(KoSvgTextShape);

    Q_UNUSED(paintContext);

    applyConversion(painter, converter);
    for (int i = 0; i < (int)d->layouts.size(); i++) {
        d->layouts[i]->draw(&painter, d->layoutOffsets[i]);
    }
}

void KoSvgTextShape::paintStroke(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(paintContext);

    // do nothing! everything is painted in paintComponent()
}

void KoSvgTextShape::resetTextShape()
{
    KoSvgTextChunkShape::resetTextShape();
    relayout();
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


void KoSvgTextShape::relayout()
{
    Q_D(KoSvgTextShape);

    d->layouts.clear();
    d->layoutOffsets.clear();

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
                line.setPosition(currentTextPos - QPointF(0, line.ascent()));
                currentTextPos.rx() += line.horizontalAdvance();


            } else {
                const int numChars = chunk.offsets[i].start - lastSubChunkStart;

                if (numChars > 0) {
                    QTextLine line = layout->createLine();
                    KIS_SAFE_ASSERT_RECOVER(line.isValid()) { break; }
                    line.setNumColumns(numChars);

                    currentTextPos += lastSubChunkOffset;
                    line.setPosition(currentTextPos - QPointF(0, line.ascent()));
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

    d->clearAssociatedOutlines(this);

    for (int i = 0; i < int(d->layouts.size()); i++) {
        const QTextLayout &layout = *d->layouts[i];
        const QPointF layoutOffset = d->layoutOffsets[i];

        using namespace KoSvgText;

        QVector<AssociatedShapeWrapper> shapeMap;
        shapeMap.resize(layout.text().size());

        Q_FOREACH (const QTextLayout::FormatRange &range, layout.additionalFormats()) {
            for (int k = range.start; k < range.start + range.length; k++) {
                KIS_SAFE_ASSERT_RECOVER_BREAK(k >= 0 && k < shapeMap.size());

                const KoSvgCharChunkFormat &format =
                    static_cast<const KoSvgCharChunkFormat&>(range.format);

                shapeMap[k] = format.associatedShapeWrapper();
            }
        }

        for (int j = 0; j < layout.lineCount(); j++) {
            const QTextLine line = layout.lineAt(j);

            for (int k = line.textStart(); k < line.textStart() + line.textLength(); k++) {
                KIS_SAFE_ASSERT_RECOVER_BREAK(k >= 0 && k < shapeMap.size());

                AssociatedShapeWrapper wrapper = shapeMap[k];
                if (!wrapper.isValid()) continue;

                const qreal x1 = line.cursorToX(k, QTextLine::Leading);
                const qreal x2 = line.cursorToX(k, QTextLine::Trailing);

                QRectF rect(qMin(x1, x2), line.y(), qAbs(x1 - x2), line.height());
                wrapper.addCharacterRect(rect.translated(layoutOffset));
            }
        }
    }
}

void KoSvgTextShapePrivate::clearAssociatedOutlines(KoShape *rootShape)
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

KoShape *KoSvgTextShapeFactory::createDefaultShape(KoDocumentResourceManager */*documentResources*/) const
{
    qDebug() << "Create default svg text shape";

    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg("<text>Lorem ipsum dolor sit amet, consectetur adipiscing elit.</text>",
                             "<defs/>",
                             QRectF(0, 0, 200, 60),
                             72.0);
    qDebug() << converter.errors() << converter.warnings();

    return shape;
}

bool KoSvgTextShapeFactory::supports(const KoXmlElement &/*e*/, KoShapeLoadingContext &/*context*/) const
{
    return false;
}
