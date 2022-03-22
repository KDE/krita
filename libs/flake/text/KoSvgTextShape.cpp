/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"

#include <QTextLayout>

#include <fontconfig/fontconfig.h>
#include <klocalizedstring.h>
#include <raqm.h>

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
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QThread>
#include <boost/optional.hpp>
#include <memory>
#include <vector>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>

#include <FlakeDebug.h>

#include <QSharedData>

struct CharacterResult {
    QPointF finalPosition;
    qreal rotate = 0.0;
    bool hidden = false;
    bool addressable = false;
    bool middle = false;
    bool anchored_chunk = false;

    QPainterPath path;
    QRectF boundingBox;
    int typographic_index = 0;
    QPointF cssPosition = QPointF();
};

class KoSvgTextShape::Private
{
public:
    // NOTE: the cache data is shared between all the instances of
    //       the shape, though it will be reset locally if the
    //       accessing thread changes
    QThread *cachedLayoutsWorkingThread = 0;
    FT_Library library = NULL;

    QVector<CharacterResult> result;

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

void KoSvgTextShape::paintComponent(QPainter &painter) const
{
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
    QTransform tf;

    for (CharacterResult cr : d->result) {
        if (cr.addressable && cr.hidden == false) {
            QPainterPath p = cr.path;
            tf.reset();
            tf.translate(cr.finalPosition.x(), cr.finalPosition.y());
            tf.rotateRadians(cr.rotate);
            background()->paint(painter, paintContext, tf.map(p));
        }
    }
    /**
     * HACK ALERT:
     * The layouts of non-gui threads must be destroyed in the same thread
     * they have been created. Because the thread might be restarted in the
     * meantime or just destroyed, meaning that the per-thread freetype data
     * will not be available.
     */
    if (QThread::currentThread() != qApp->thread()) {
        d->cachedLayoutsWorkingThread = 0;
    }
}

void KoSvgTextShape::paintStroke(QPainter &painter) const
{
    Q_UNUSED(painter);
    // do nothing! everything is painted in paintComponent()
}

QPainterPath KoSvgTextShape::textOutline() const
{
    /*QPainterPath result;
    result.setFillRule(Qt::WindingFill);
    qDebug() << "starting creation textoutlne";

    for (int layoutIndex = 0; layoutIndex < (int)d->cachedLayouts.size();
    layoutIndex++) {

        for (int j = 0; j < layout->lineCount(); j++) {
            QTextLine line = layout->lineAt(j);

            Q_FOREACH (const QGlyphRun &run, line.glyphRuns()) {
                const QVector<quint32> indexes = run.glyphIndexes();
                const QVector<QPointF> positions = run.positions();
                const QRawFont font = run.rawFont();

                KIS_SAFE_ASSERT_RECOVER(indexes.size() == positions.size()) {
    continue; }

                for (int k = 0; k < indexes.size(); k++) {
                    QPainterPath glyph = font.pathForGlyph(indexes[k]);
                    glyph.translate(positions[k] + layoutOffset);
                    result += glyph;
                }

                const qreal thickness = font.lineThickness();
                const QRectF runBounds = run.boundingRect();

                if (run.overline()) {
                    // the offset is calculated to be consistent with the way
    how Qt renders the text const qreal y = line.y(); QRectF
    overlineBlob(runBounds.x(), y, runBounds.width(), thickness);
                    overlineBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(overlineBlob);

                    // don't use direct addRect, because it doesn't care about
    Qt::WindingFill result += path;
                }

                if (run.strikeOut()) {
                    // the offset is calculated to be consistent with the way
    how Qt renders the text const qreal y = line.y() + 0.5 * line.height();
                    QRectF strikeThroughBlob(runBounds.x(), y,
    runBounds.width(), thickness); strikeThroughBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(strikeThroughBlob);

                    // don't use direct addRect, because it doesn't care about
    Qt::WindingFill result += path;
                }

                if (run.underline()) {
                    const qreal y = line.y() + line.ascent() +
    font.underlinePosition(); QRectF underlineBlob(runBounds.x(), y,
    runBounds.width(), thickness); underlineBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(underlineBlob);

                    // don't use direct addRect, because it doesn't care about
    Qt::WindingFill result += path;
                }
            }
        }
    }*/

    return QPainterPath();
}

void KoSvgTextShape::resetTextShape()
{
    KoSvgTextChunkShape::resetTextShape();
    relayout();
}

void KoSvgTextShape::relayout() const
{
    d->cachedLayoutsWorkingThread = QThread::currentThread();
    // Calculate the associated outline for a text chunk.
    d->clearAssociatedOutlines(this);

    // The following is based on the text-layout algorithm in SVG 2.
    KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(
        this->textProperties()
            .propertyOrDefault(KoSvgTextProperties::WritingModeId)
            .toInt());

    // first, get text. We use the subChunks because that handles bidi for us.
    // SVG 1.1 suggests that each time the xy position of a piece of text
    // changes, that this should be seperately shaped, but according to SVGWG
    // issues 631 and 635 noone who actually uses bidi likes this, and it also
    // complicates the alorithm, so we're getting rid of this.
    // https://github.com/w3c/svgwg/issues/631
    // https://github.com/w3c/svgwg/issues/635

    QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> textChunks =
        layoutInterface()->collectSubChunks();
    QString text;
    for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
        text.append(chunk.text);
    }

    // Then, pass everything to a css-compatible text-layout algortihm.
    raqm_t *layout(raqm_create());

    if (!d->library) {
        FT_Init_FreeType(&d->library);
    }
    FcConfig *config = FcConfigGetCurrent();
    FcObjectSet *objectSet = FcObjectSetBuild(FC_FAMILY, FC_FILE, nullptr);

    QMap<QString, FT_Face> faces;

    if (raqm_set_text_utf8(layout, text.toUtf8(), text.toUtf8().size())) {
        if (writingMode == KoSvgText::TopToBottom) {
            raqm_set_par_direction(layout,
                                   raqm_direction_t::RAQM_DIRECTION_TTB);
        } else if (writingMode == KoSvgText::RightToLeft) {
            raqm_set_par_direction(layout,
                                   raqm_direction_t::RAQM_DIRECTION_RTL);
        } else {
            raqm_set_par_direction(layout,
                                   raqm_direction_t::RAQM_DIRECTION_LTR);
        }

        int start = 0;
        int length = 0;
        for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
            length = chunk.text.toUtf8().size();
            FT_Face face = NULL;

            FcPattern *p = FcPatternCreate();
            const FcChar8 *vals = reinterpret_cast<FcChar8 *>(
                chunk.format.font().family().toUtf8().data());
            qreal fontSize = chunk.format.font().pointSizeF();
            FcPatternAddString(p, FC_FAMILY, vals);
            /* this is too strict, and will sometimes prevent
                 * fonts to be found, so we'll need to handle this
               differently... if (range.format.font().italic()) {
                    FcPatternAddInteger(p, FC_SLANT, FC_SLANT_ITALIC);
                }
                //The following is wrong, but it'll have to do for now...
                FcPatternAddInteger(p, FC_WEIGHT,
               range.format.font().weight()*2); if
               (range.format.font().stretch() >= 50) { FcPatternAddInteger(p,
               FC_WIDTH, range.format.font().stretch());
                }*/
            FcFontSet *fontSet = FcFontList(config, p, objectSet);
            if (fontSet->nfont == 0) {
                qWarning() << "No fonts found for family name"
                           << chunk.format.font().family();
                continue;
            }
            QString fontFileName;
            QStringList fontProperties =
                QString(
                    reinterpret_cast<char *>(FcNameUnparse(fontSet->fonts[0])))
                    .split(':');
            for (QString value : fontProperties) {
                if (value.startsWith("file")) {
                    fontFileName = value.split("=").last();
                    fontFileName.remove("\\");
                }
            }

            int errorCode =
                FT_New_Face(d->library, fontFileName.toUtf8().data(), 0, &face);
            if (errorCode == 0) {
                qDebug() << "face loaded" << fontFileName;
                errorCode = FT_Set_Char_Size(face, fontSize * 64.0, 0, 0, 0);
                if (errorCode == 0) {
                    if (start == 0) {
                        raqm_set_freetype_face(layout, face);
                    }
                    if (length > 0) {
                        raqm_set_freetype_face_range(layout,
                                                     face,
                                                     start,
                                                     length);
                    }
                }
            } else {
                qDebug() << "Face did not load, FreeType Error: " << errorCode
                         << "Filename:" << fontFileName;
            }
            FT_Done_Face(face);
            start += length;
        }
        qDebug() << "text-length:" << text.toUtf8().size();
    }

    if (raqm_layout(layout)) {
        qDebug() << "layout succeeded";
    }

    // 1. Setup.

    QVector<CharacterResult> result(text.toUtf8().size());
    bool isHorizontal = true;
    if (writingMode == KoSvgText::TopToBottom) {
        isHorizontal = false;
    }

    // 2. Set flags and assign initial positions
    // We also retreive a path here.
    size_t count;
    raqm_glyph_t *glyphs = raqm_get_glyphs(layout, &count);
    if (!glyphs) {
        return;
    }
    QVector<int> addressableIndices;

    QTransform ftTF;
    const qreal factor = 1 / 64.;
    // This is dependant on the writing mode, but it seems also
    // dependant on whether the fontface was loaded with vertical metrics...
    ftTF.scale(factor, -factor);

    QPointF totalAdvance;

    for (int g = 0; g < int(count); g++) {
        int error = FT_Load_Glyph(glyphs[g].ftface, glyphs[g].index, 0);
        if (error != 0) {
            continue;
        }
        FT_GlyphSlotRec *glyphSlot = glyphs[g].ftface->glyph;

        qDebug() << "glyph" << g << "cluster" << glyphs[g].cluster;
        // FT_Glyph_Get_CBox( glyphs[i].ftface->glyph, 0, &bbox );

        QPointF cp = QPointF();
        // convert the outline to a painter path
        QPainterPath glyph;
        int i = 0;
        for (int j = 0; j < glyphSlot->outline.n_contours; ++j) {
            int last_point = glyphSlot->outline.contours[j];
            // qDebug() << "contour:" << i << "to" << last_point;
            QPointF start = QPointF(glyphSlot->outline.points[i].x,
                                    glyphSlot->outline.points[i].y);
            if (!(glyphSlot->outline.tags[i]
                  & 1)) { // start point is not on curve:
                if (!(glyphSlot->outline.tags[last_point]
                      & 1)) { // end point is not on curve:
                    // qDebug() << "  start and end point are not on curve";
                    start = (QPointF(glyphSlot->outline.points[last_point].x,
                                     glyphSlot->outline.points[last_point].y)
                             + start)
                        / 2.0;
                } else {
                    // qDebug() << "  end point is on curve, start is not";
                    start = QPointF(glyphSlot->outline.points[last_point].x,
                                    glyphSlot->outline.points[last_point].y);
                }
                --i; // to use original start point as control point below
            }
            start += cp;
            // qDebug() << "  start at" << start;
            glyph.moveTo(start);
            QPointF c[4];
            c[0] = start;
            int n = 1;
            while (i < last_point) {
                ++i;
                c[n] = cp
                    + QPointF(glyphSlot->outline.points[i].x,
                              glyphSlot->outline.points[i].y);
                // qDebug() << "    " << i << c[n] << "tag =" <<
                // (int)g->outline.tags[i]
                //                    << ": on curve =" <<
                //                    (bool)(g->outline.tags[i] & 1);
                ++n;
                switch (glyphSlot->outline.tags[i] & 3) {
                case 2:
                    // cubic bezier element
                    if (n < 4)
                        continue;
                    c[3] = (c[3] + c[2]) / 2;
                    --i;
                    break;
                case 0:
                    // quadratic bezier element
                    if (n < 3)
                        continue;
                    c[3] = (c[1] + c[2]) / 2;
                    c[2] = (2 * c[1] + c[3]) / 3;
                    c[1] = (2 * c[1] + c[0]) / 3;
                    --i;
                    break;
                case 1:
                case 3:
                    if (n == 2) {
                        // qDebug() << "  lineTo" << c[1];
                        glyph.lineTo(c[1]);
                        c[0] = c[1];
                        n = 1;
                        continue;
                    } else if (n == 3) {
                        c[3] = c[2];
                        c[2] = (2 * c[1] + c[3]) / 3;
                        c[1] = (2 * c[1] + c[0]) / 3;
                    }
                    break;
                }
                // qDebug() << "  cubicTo" << c[1] << c[2] << c[3];
                glyph.cubicTo(c[1], c[2], c[3]);
                c[0] = c[3];
                n = 1;
            }
            if (n == 1) {
                // qDebug() << "  closeSubpath";
                glyph.closeSubpath();
            } else {
                c[3] = start;
                if (n == 2) {
                    c[2] = (2 * c[1] + c[3]) / 3;
                    c[1] = (2 * c[1] + c[0]) / 3;
                }
                // qDebug() << "  close cubicTo" << c[1] << c[2] << c[3];
                glyph.cubicTo(c[1], c[2], c[3]);
            }
            ++i;
        }
        glyph = ftTF.map(glyph);
        QPointF advance(glyphs[g].x_advance, glyphs[g].y_advance);
        advance = ftTF.map(advance);
        QPointF offset(glyphs[g].x_offset, glyphs[g].y_offset);
        offset = ftTF.map(offset);

        CharacterResult charResult;
        charResult.path = glyph;
        if (glyph.isEmpty()) {
            charResult.boundingBox = QRectF(QPointF(), advance);
        } else {
            charResult.boundingBox = glyph.boundingRect();
        }
        charResult.typographic_index = glyphs[g].index;
        charResult.addressable = true;
        addressableIndices.append(glyphs[g].cluster);
        // if character in middle of line, this doesn't mean much rght now,
        // because we don't do linebreaking yet, but once we do, we should set
        // these appropriately.
        if (g == 0) {
            charResult.anchored_chunk = true;
        } else {
            charResult.middle = true;
        }
        charResult.cssPosition = totalAdvance + offset;
        /**
          There's a weird note in the algorithm here that I do not understand:
          "if addressable is true and middle is false, then set the css position
          to the corresponding typographic character as determined by the css
          renderer. Otherwise if i>0, then set css possition[i] to css position
          [i-1]."
          */

        result[glyphs[g].cluster] = charResult;
        totalAdvance += advance;
    }
    // we're done with raqm for now.
    raqm_destroy(layout);

    // 3. Resolve character positioning
    qDebug() << "character positing";
    QVector<KoSvgText::CharTransformation> resolvedTransforms(
        text.toUtf8().size());
    bool textInPath = false;
    int globalIndex = 0;
    this->layoutInterface()->resolveCharacterPositioning(addressableIndices,
                                                         resolvedTransforms,
                                                         textInPath,
                                                         globalIndex,
                                                         isHorizontal);
    // 4. Adjust positions: dx, dy

    QPointF shift = QPointF();
    for (int i = 0; i < result.size(); i++) {
        if (addressableIndices.contains(i)) {
            KoSvgText::CharTransformation transform = resolvedTransforms[i];
            if (transform.hasRelativeOffset()) {
                shift += transform.relativeOffset();
            }
            CharacterResult charResult = result[i];
            if (transform.rotate) {
                charResult.rotate = *transform.rotate;
            }
            charResult.finalPosition = charResult.cssPosition + shift;
            result[i] = charResult;
        }
    }

    // 5. Apply ‘textLength’ attribute
    // 6. Adjust positions: x, y

    shift = QPointF();
    bool setNextAnchor = false;
    for (int i = 1; i < result.size(); i++) {
        if (addressableIndices.contains(i)) {
            KoSvgText::CharTransformation transform = resolvedTransforms[i];
            CharacterResult charResult = result[i];
            if (transform.xPos) {
                shift.setX(*transform.xPos - charResult.finalPosition.x());
                charResult.anchored_chunk = true;
            }
            if (transform.yPos) {
                shift.setY(*transform.yPos - charResult.finalPosition.y());
                charResult.anchored_chunk = true;
            }

            charResult.finalPosition += shift;

            if (setNextAnchor) {
                charResult.anchored_chunk = true;
            }

            if (charResult.middle && charResult.anchored_chunk) {
                charResult.anchored_chunk = false;
                if (i + 1 < result.size()) {
                    setNextAnchor = true;
                } else {
                    setNextAnchor = false;
                }
            }

            result[i] = charResult;
        }
    }

    // 7. Apply anchoring
    // 8. Position on path

    // 9. return result.
    d->result = result;
    QTransform tf;
    KoSvgText::AssociatedShapeWrapper wrapper =
        textChunks.at(0).format.associatedShapeWrapper();
    for (CharacterResult cr : d->result) {
        if (cr.addressable && cr.hidden == false) {
            tf.reset();
            tf.translate(cr.finalPosition.x(), cr.finalPosition.y());
            tf.rotateRadians(cr.rotate);
            wrapper.addCharacterRect(tf.mapRect(cr.boundingBox));
        }
    }
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
