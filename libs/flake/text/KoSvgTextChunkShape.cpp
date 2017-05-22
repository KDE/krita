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

#include "KoSvgTextChunkShape.h"
#include "KoSvgTextChunkShape_p.h"

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"


#include "kis_debug.h"
#include <KoXmlReader.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>


namespace {
QVector<qreal> parseListAttributeX(const QString &value, SvgLoadingContext &context)
{
    QVector<qreal> result;

    QStringList list = SvgUtil::simplifyList(value);
    Q_FOREACH (const QString &str, list) {
        result << SvgUtil::parseUnitX(context.currentGC(), str);
    }

    return result;
}

QVector<qreal> parseListAttributeY(const QString &value, SvgLoadingContext &context)
{
    QVector<qreal> result;

    QStringList list = SvgUtil::simplifyList(value);
    Q_FOREACH (const QString &str, list) {
        result << SvgUtil::parseUnitY(context.currentGC(), str);
    }

    return result;
}

QVector<qreal> parseListAttributeAngular(const QString &value, SvgLoadingContext &context)
{
    QVector<qreal> result;

    QStringList list = SvgUtil::simplifyList(value);
    Q_FOREACH (const QString &str, list) {
        result << SvgUtil::parseUnitAngular(context.currentGC(), str);
    }

    return result;
}
}

struct KoSvgTextChunkShapePrivate::LayoutInterface : public KoSvgTextChunkShapeLayoutInterface
{
    LayoutInterface(KoSvgTextChunkShape *_q) : q(_q) {}

    KoSvgText::AutoValue textLength() const {
        return q->d_func()->textLength;
    }

    KoSvgText::LengthAdjust lengthAdjust() const {
        return q->d_func()->lengthAdjust;
    }

    int numChars() const {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->d_func()->text.isEmpty(), 0);

        int result = 0;

        if (!q->shapeCount()) {
            result = q->d_func()->text.size();
        } else {
            Q_FOREACH (KoShape *shape, q->shapes()) {
                KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(chunkShape, 0);
                result += chunkShape->layoutInterface()->numChars();
            }
        }

        return result;
    }

    int relativeCharPos(KoSvgTextChunkShape *child, int pos) const {
        QList<KoShape*> childShapes = q->shapes();

        int result = -1;
        int numCharsPassed = 0;

        Q_FOREACH (KoShape *shape, q->shapes()) {
            KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(chunkShape, 0);

            if (chunkShape == child) {
                result = pos + numCharsPassed;
                break;
            } else {
                numCharsPassed += chunkShape->layoutInterface()->numChars();
            }

        }

        return result;
    }

    bool isTextNode() const {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->d_func()->text.isEmpty(), false);
        return !q->shapeCount();
    }

    QString nodeText() const {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->d_func()->text.isEmpty(), 0);
        return !q->shapeCount() ? q->d_func()->text : QString();
    }

    QVector<KoSvgText::CharTransformation> localCharTransformations() const {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(isTextNode(), QVector<KoSvgText::CharTransformation>());

        const QVector<KoSvgText::CharTransformation> t = q->d_func()->localTransformations;
        return t.mid(0, qMin(t.size(), q->d_func()->text.size()));
    }

    static QString getBidiOpening(KoSvgText::Direction direction, KoSvgText::UnicodeBidi bidi) {
        using namespace KoSvgText;

        QString result;

        if (bidi == BidiEmbed) {
            result = direction == DirectionLeftToRight ? "\u202a" : "\u202b";
        } else if (bidi == BidiOverride) {
            result = direction == DirectionLeftToRight ? "\u202d" : "\u202e";
        }

        return result;
    }

    QVector<SubChunk> collectSubChunks() const {
        QVector<SubChunk> result;

        if (isTextNode()) {
            const QString text = q->d_func()->text;
            const KoSvgText::KoSvgCharChunkFormat format = q->d_func()->fetchCharFormat();
            const QVector<KoSvgText::CharTransformation> transforms = q->d_func()->localTransformations;

            KoSvgText::UnicodeBidi bidi = KoSvgText::UnicodeBidi(q->d_func()->properties.propertyOrDefault(KoSvgTextProperties::UnicodeBidiId).toInt());
            KoSvgText::Direction direction = KoSvgText::Direction(q->d_func()->properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
            const QString bidiOpening = getBidiOpening(direction, bidi);

            if (!bidiOpening.isEmpty()) {
                result << SubChunk(bidiOpening, format);
            }

            if (transforms.isEmpty()) {
                result << SubChunk(text, format);
            } else {
                for (int i = 0; i < transforms.size(); i++) {
                    const KoSvgText::CharTransformation baseTransform = transforms[i];
                    int subChunkLength = 1;

                    for (int j = i + 1; j < transforms.size(); j++) {
                        if (transforms[j] == baseTransform) {
                            subChunkLength++;
                        }
                    }

                    if (i + subChunkLength >= transforms.size()) {
                        subChunkLength = text.size() - i;
                    }

                    result << SubChunk(text.mid(i, subChunkLength), format, baseTransform);
                    i += subChunkLength - 1;
                }

            }

            if (!bidiOpening.isEmpty()) {
                result << SubChunk("\u202c", format);
            }

        } else {
            Q_FOREACH (KoShape *shape, q->shapes()) {
                KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
                KIS_SAFE_ASSERT_RECOVER_BREAK(chunkShape);

                result += chunkShape->layoutInterface()->collectSubChunks();
            }
        }

        return result;
    }

private:
    KoSvgTextChunkShape *q;
};

KoSvgTextChunkShape::KoSvgTextChunkShape()
    : KoShapeContainer(new KoSvgTextChunkShapePrivate(this))
{
    Q_D(KoSvgTextChunkShape);
    d->layoutInterface.reset(new KoSvgTextChunkShapePrivate::LayoutInterface(this));
}

KoSvgTextChunkShape::KoSvgTextChunkShape(const KoSvgTextChunkShape &rhs)
    : KoShapeContainer(new KoSvgTextChunkShapePrivate(*rhs.d_func(), this))
{
    Q_D(KoSvgTextChunkShape);
    d->layoutInterface.reset(new KoSvgTextChunkShapePrivate::LayoutInterface(this));
}

KoSvgTextChunkShape::KoSvgTextChunkShape(KoSvgTextChunkShapePrivate *dd)
    : KoShapeContainer(dd)
{
    Q_D(KoSvgTextChunkShape);
    d->layoutInterface.reset(new KoSvgTextChunkShapePrivate::LayoutInterface(this));
}

KoSvgTextChunkShape::~KoSvgTextChunkShape()
{
}

KoShape *KoSvgTextChunkShape::cloneShape() const
{
    return new KoSvgTextChunkShape(*this);
}


void KoSvgTextChunkShape::paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(paintContext);
}

void KoSvgTextChunkShape::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

bool KoSvgTextChunkShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return false;
}

bool KoSvgTextChunkShape::saveSvg(SvgSavingContext &context)
{
    Q_UNUSED(context);
    return true;
}


void KoSvgTextChunkShapePrivate::loadContextBasedProperties(SvgGraphicsContext *gc)
{
    properties = gc->textProperties;
    font = gc->font;
    fontFamiliesList = gc->fontFamiliesList;
}

bool KoSvgTextChunkShape::loadSvg(const KoXmlElement &e, SvgLoadingContext &context)
{
    Q_D(KoSvgTextChunkShape);

    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    d->loadContextBasedProperties(gc);

    d->textLength = KoSvgText::parseAutoValueXY(e.attribute("textLength", ""), context, "");
    d->lengthAdjust = KoSvgText::parseLengthAdjust(e.attribute("lengthAdjust", "spacing"));

    QVector<qreal> xPos = parseListAttributeX(e.attribute("x", ""), context);
    QVector<qreal> yPos = parseListAttributeY(e.attribute("y", ""), context);
    QVector<qreal> dxPos = parseListAttributeX(e.attribute("dx", ""), context);
    QVector<qreal> dyPos = parseListAttributeY(e.attribute("dy", ""), context);
    QVector<qreal> rotate = parseListAttributeAngular(e.attribute("rotate", ""), context);

    const int numLocalTransformations =
        std::max({xPos.size(), yPos.size(),
                  dxPos.size(), dyPos.size(),
                  rotate.size()});

    d->localTransformations.resize(numLocalTransformations);
    for (int i = 0; i < numLocalTransformations; i++) {
        if (i < xPos.size()) {
            d->localTransformations[i].xPos = xPos[i];
        }
        if (i < yPos.size()) {
            d->localTransformations[i].yPos = yPos[i];
        }
        if (i < dxPos.size() && dxPos[i] != 0.0) {
            d->localTransformations[i].dxPos = dxPos[i];
        }
        if (i < dyPos.size() && dyPos[i] != 0.0) {
            d->localTransformations[i].dyPos = dyPos[i];
        }
        if (i < rotate.size()) {
            d->localTransformations[i].rotate = rotate[i];
        }
    }

    Q_UNUSED(e);
    Q_UNUSED(context);
    return true;
}

bool KoSvgTextChunkShape::loadSvgTextNode(const KoXmlText &text, SvgLoadingContext &context)
{
    Q_D(KoSvgTextChunkShape);

    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    d->loadContextBasedProperties(gc);

    QString data = text.data();

    data.replace(QRegExp("[\\r\\n]"), "");
    data.replace(QRegExp("\\s{2,}"), " ");

    if (text.previousSibling().isNull() && data.startsWith(' ')) {
        data.remove(0, 1);
    }

    if (text.nextSibling().isNull() && data.endsWith(' ')) {
        data.remove(data.size() - 1, 1);
    }

    d->text = data;

    return !data.isEmpty();
}

void KoSvgTextChunkShape::normalizeCharTransformations()
{
    Q_D(KoSvgTextChunkShape);
    d->applyParentCharTransformations(d->localTransformations);
}

KoSvgTextProperties KoSvgTextChunkShape::textProperties() const
{
    Q_D(const KoSvgTextChunkShape);
    return d->properties;
}

bool KoSvgTextChunkShape::isTextNode() const
{
    Q_D(const KoSvgTextChunkShape);
    return d->layoutInterface->isTextNode();
}

KoSvgTextChunkShapeLayoutInterface *KoSvgTextChunkShape::layoutInterface()
{
    Q_D(KoSvgTextChunkShape);
    return d->layoutInterface.data();
}

/**************************************************************************************************/
/*                 KoSvgTextChunkShapePrivate                                                     */
/**************************************************************************************************/

KoSvgTextChunkShapePrivate::KoSvgTextChunkShapePrivate(KoSvgTextChunkShape *_q)
    : KoShapeContainerPrivate(_q)
{
}

KoSvgTextChunkShapePrivate::KoSvgTextChunkShapePrivate(const KoSvgTextChunkShapePrivate &rhs, KoSvgTextChunkShape *q)
    : KoShapeContainerPrivate(rhs, q),
      properties(rhs.properties)
{
}

KoSvgTextChunkShapePrivate::~KoSvgTextChunkShapePrivate()
{
}

KoSvgText::KoSvgCharChunkFormat KoSvgTextChunkShapePrivate::fetchCharFormat() const
{
    Q_Q(const KoSvgTextChunkShape);

    KoSvgText::KoSvgCharChunkFormat format;

    format.setFont(font);
    format.setTextAnchor(KoSvgText::TextAnchor(properties.propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt()));

    KoSvgText::Direction direction =
        KoSvgText::Direction(properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    format.setLayoutDirection(direction == KoSvgText::DirectionLeftToRight ? Qt::LeftToRight : Qt::RightToLeft);


    KoSvgText::BaselineShiftMode shiftMode =
        KoSvgText::BaselineShiftMode(properties.propertyOrDefault(KoSvgTextProperties::BaselineShiftModeId).toInt());

    // FIXME: we support only 'none', 'sub' and 'super' shifts at the moment.
    //        Please implement 'percentage' as well!

    // WARNING!!! Qt's setVerticalAlignment() also changes the size of the font! And SVG does not(!) imply it!
    if (shiftMode == KoSvgText::ShiftSub) {
        format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    } else if (shiftMode == KoSvgText::ShiftSuper) {
        format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    }

    KoSvgText::AutoValue letterSpacing = properties.propertyOrDefault(KoSvgTextProperties::LetterSpacingId).value<KoSvgText::AutoValue>();
    if (!letterSpacing.isAuto) {
        format.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        format.setFontLetterSpacing(letterSpacing.customValue);
    }

    KoSvgText::AutoValue wordSpacing = properties.propertyOrDefault(KoSvgTextProperties::WordSpacingId).value<KoSvgText::AutoValue>();
    if (!wordSpacing.isAuto) {
        format.setFontWordSpacing(wordSpacing.customValue);
    }

    KoSvgText::AutoValue kerning = properties.propertyOrDefault(KoSvgTextProperties::KerningId).value<KoSvgText::AutoValue>();
    if (!kerning.isAuto) {
        format.setFontKerning(false);
        format.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        format.setFontLetterSpacing(format.fontLetterSpacing() + kerning.customValue);
    }

    return format;
}

void KoSvgTextChunkShapePrivate::applyParentCharTransformations(const QVector<KoSvgText::CharTransformation> transformations)
{
    Q_Q(KoSvgTextChunkShape);

    if (q->shapeCount()) {
        int numCharsPassed = 0;

        Q_FOREACH (KoShape *shape, q->shapes()) {
            KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
            KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

            const int numCharsInSubtree = chunkShape->layoutInterface()->numChars();
            QVector<KoSvgText::CharTransformation> t = transformations.mid(numCharsPassed, numCharsInSubtree);
            if (t.isEmpty()) break;

            chunkShape->d_func()->applyParentCharTransformations(t);
            numCharsPassed += numCharsInSubtree;

            if (numCharsPassed >= transformations.size()) break;
        }
    } else {
        for (int i = 0; i < qMin(transformations.size(), text.size()); i++) {
            KIS_SAFE_ASSERT_RECOVER_RETURN(localTransformations.size() >= i);

            if (localTransformations.size() == i) {
                localTransformations.append(transformations[i]);
            } else {
                localTransformations[i].mergeInParentTransformation(transformations[i]);
            }
        }
    }
}
