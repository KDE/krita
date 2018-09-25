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
#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>

#include <SvgSavingContext.h>
#include <SvgStyleWriter.h>
#include <kis_dom_utils.h>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>
#include <commands/KoShapeUngroupCommand.h>

#include <html/HtmlSavingContext.h>

#include <FlakeDebug.h>

namespace {


void appendLazy(QVector<qreal> *list, boost::optional<qreal> value, int iteration, bool hasDefault = true, qreal defaultValue = 0.0)
{
    if (!value) return;
    if (value && *value == defaultValue && hasDefault == true && list->isEmpty()) return;

    while (list->size() < iteration) {
        list->append(defaultValue);
    }

    list->append(*value);
}

void fillTransforms(QVector<qreal> *xPos, QVector<qreal> *yPos, QVector<qreal> *dxPos, QVector<qreal> *dyPos, QVector<qreal> *rotate,
                    QVector<KoSvgText::CharTransformation> localTransformations)
{
    for (int i = 0; i < localTransformations.size(); i++) {
        const KoSvgText::CharTransformation &t = localTransformations[i];
        appendLazy(xPos, t.xPos, i, false);
        appendLazy(yPos, t.yPos, i, false);
        appendLazy(dxPos, t.dxPos, i);
        appendLazy(dyPos, t.dyPos, i);
        appendLazy(rotate, t.rotate, i);
    }
}

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

QString convertListAttribute(const QVector<qreal> &values) {
    QStringList stringValues;

    Q_FOREACH (qreal value, values) {
        stringValues.append(KisDomUtils::toString(value));
    }

    return stringValues.join(',');
}
}

struct KoSvgTextChunkShapePrivate::LayoutInterface : public KoSvgTextChunkShapeLayoutInterface
{
    LayoutInterface(KoSvgTextChunkShape *_q) : q(_q) {}

    KoSvgText::AutoValue textLength() const override {
        return q->d_func()->textLength;
    }

    KoSvgText::LengthAdjust lengthAdjust() const override {
        return q->d_func()->lengthAdjust;
    }

    int numChars() const override {
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

    int relativeCharPos(KoSvgTextChunkShape *child, int pos) const override {
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

    bool isTextNode() const override {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->d_func()->text.isEmpty(), false);
        return !q->shapeCount();
    }

    QString nodeText() const override {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->d_func()->text.isEmpty(), 0);
        return !q->shapeCount() ? q->d_func()->text : QString();
    }

    QVector<KoSvgText::CharTransformation> localCharTransformations() const override {
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

    QVector<SubChunk> collectSubChunks() const override {
        QVector<SubChunk> result;

        if (isTextNode()) {
            const QString text = q->d_func()->text;
            const KoSvgText::KoSvgCharChunkFormat format = q->d_func()->fetchCharFormat();
            QVector<KoSvgText::CharTransformation> transforms = q->d_func()->localTransformations;

            /**
             * Sometimes SVG can contain the X,Y offsets for the pieces of text that
             * do not exist, just skip them.
             */
            if (text.size() <= transforms.size()) {
                transforms.resize(text.size());
            }

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
                        if (transforms[j].isNull()) {
                            subChunkLength++;
                        } else {
                            break;
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

    void addAssociatedOutline(const QRectF &rect) override {
        KIS_SAFE_ASSERT_RECOVER_RETURN(isTextNode());
        QPainterPath path;
        path.addRect(rect);
        path |= q->d_func()->associatedOutline;
        path.setFillRule(Qt::WindingFill);
        path = path.simplified();

        q->d_func()->associatedOutline = path;
        q->d_func()->size = path.boundingRect().size();

        q->notifyChanged();
        q->d_func()->shapeChanged(KoShape::SizeChanged);
    }

    void clearAssociatedOutline() override {
        q->d_func()->associatedOutline = QPainterPath();
        q->d_func()->size = QSizeF();

        q->notifyChanged();
        q->d_func()->shapeChanged(KoShape::SizeChanged);
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

QSizeF KoSvgTextChunkShape::size() const
{
    return outlineRect().size();
}

void KoSvgTextChunkShape::setSize(const QSizeF &size)
{
    Q_UNUSED(size);
    // we do not support resizing!
}

QRectF KoSvgTextChunkShape::outlineRect() const
{
    return outline().boundingRect();
}

QPainterPath KoSvgTextChunkShape::outline() const
{
    Q_D(const KoSvgTextChunkShape);

    QPainterPath result;
    result.setFillRule(Qt::WindingFill);

    if (d->layoutInterface->isTextNode()) {
        result = d->associatedOutline;
    } else {
        Q_FOREACH (KoShape *shape, shapes()) {
            KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
            KIS_SAFE_ASSERT_RECOVER_BREAK(chunkShape);

            result |= chunkShape->outline();
        }
    }

    return result.simplified();
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

bool KoSvgTextChunkShape::saveHtml(HtmlSavingContext &context)
{
    Q_D(KoSvgTextChunkShape);

    // Should we add a newline? Check for vertical movement if we're using rtl or ltr text
    // XXX: if vertical text, check horizontal movement.
    QVector<qreal> xPos;
    QVector<qreal> yPos;
    QVector<qreal> dxPos;
    QVector<qreal> dyPos;
    QVector<qreal> rotate;

    fillTransforms(&xPos, &yPos, &dxPos, &dyPos, &rotate, d->localTransformations);

    for (int i = 0; i < d->localTransformations.size(); i++) {
        const KoSvgText::CharTransformation &t = d->localTransformations[i];

        appendLazy(&xPos, t.xPos, i, false);
        appendLazy(&yPos, t.yPos, i, false);
        appendLazy(&dxPos, t.dxPos, i);
        appendLazy(&dyPos, t.dyPos, i);
    }

    KoSvgTextChunkShape *parent = !isRootTextNode() ? dynamic_cast<KoSvgTextChunkShape*>(this->parent()) : 0;
    KoSvgTextProperties parentProperties =
        parent ? parent->textProperties() : KoSvgTextProperties::defaultProperties();

    // XXX: we don't save fill, stroke, text length, length adjust or spacing and glyphs.
    KoSvgTextProperties ownProperties = textProperties().ownProperties(parentProperties);

    if (isRootTextNode()) {
        context.shapeWriter().startElement("body", false);
        if (layoutInterface()->isTextNode()) {
            context.shapeWriter().startElement("p", false);
        }
        // XXX: Save the style?

    } else if (parent->isRootTextNode()) {
        context.shapeWriter().startElement("p", false);
    } else {
        context.shapeWriter().startElement("span", false);
        // XXX: Save the style?
    }

    QMap<QString, QString> attributes = ownProperties.convertToSvgTextAttributes();
    if (attributes.size() > 0) {
        QString styleString;
        for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
            if (QString(it.key().toLatin1().data()).contains("text-anchor")) {
                QString val = it.value();
                if (it.value()=="middle") {
                    val = "center";
                } else if (it.value()=="end") {
                    val = "right";
                } else {
                    val = "left";
                }
                styleString.append("text-align")
                        .append(": ")
                        .append(val)
                        .append(";" );
            } else if (QString(it.key().toLatin1().data()).contains("fill")){
                styleString.append("color")
                        .append(": ")
                        .append(it.value())
                        .append(";" );
            } else if (QString(it.key().toLatin1().data()).contains("font-size")){
                QString val = it.value();
                if (QRegExp ("\\d*").exactMatch(val)) {
                    val.append("pt");
                }
                styleString.append(it.key().toLatin1().data())
                        .append(": ")
                        .append(val)
                        .append(";" );
            } else {
                styleString.append(it.key().toLatin1().data())
                        .append(": ")
                        .append(it.value())
                        .append(";" );
            }
        }
        context.shapeWriter().addAttribute("style", styleString);
    }
    if (layoutInterface()->isTextNode()) {
        debugFlake << "saveHTML" << this << d->text << xPos << yPos << dxPos << dyPos;
        // After adding all the styling to the <p> element, add the text
        context.shapeWriter().addTextNode(d->text);
    }
    else {
        Q_FOREACH (KoShape *child, this->shapes()) {
            KoSvgTextChunkShape *childText = dynamic_cast<KoSvgTextChunkShape*>(child);
            KIS_SAFE_ASSERT_RECOVER(childText) { continue; }
            childText->saveHtml(context);
        }
    }

    if (isRootTextNode() && layoutInterface()->isTextNode()) {
        context.shapeWriter().endElement(); // body
    }
    context.shapeWriter().endElement(); // p or span

    return true;
}

void writeTextListAttribute(const QString &attribute, const QVector<qreal> &values, KoXmlWriter &writer)
{
    const QString value = convertListAttribute(values);
    if (!value.isEmpty()) {
        writer.addAttribute(attribute.toLatin1().data(), value);
    }
}


bool KoSvgTextChunkShape::saveSvg(SvgSavingContext &context)
{
    Q_D(KoSvgTextChunkShape);

    if (isRootTextNode()) {
        context.shapeWriter().startElement("text", false);

        if (!context.strippedTextMode()) {
            context.shapeWriter().addAttribute("id", context.getID(this));
            SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
            SvgStyleWriter::saveSvgStyle(this, context);
        } else {
            SvgStyleWriter::saveSvgFill(this, context);
            SvgStyleWriter::saveSvgStroke(this, context);
        }
    } else {
        context.shapeWriter().startElement("tspan", false);
        if (!context.strippedTextMode()) {
            SvgStyleWriter::saveSvgBasicStyle(this, context);
        }
    }

    if (layoutInterface()->isTextNode()) {

        QVector<qreal> xPos;
        QVector<qreal> yPos;
        QVector<qreal> dxPos;
        QVector<qreal> dyPos;
        QVector<qreal> rotate;

        fillTransforms(&xPos, &yPos, &dxPos, &dyPos, &rotate, d->localTransformations);

        writeTextListAttribute("x", xPos, context.shapeWriter());
        writeTextListAttribute("y", yPos, context.shapeWriter());
        writeTextListAttribute("dx", dxPos, context.shapeWriter());
        writeTextListAttribute("dy", dyPos, context.shapeWriter());
        writeTextListAttribute("rotate", rotate, context.shapeWriter());
    }

    if (!d->textLength.isAuto) {
        context.shapeWriter().addAttribute("textLength", KisDomUtils::toString(d->textLength.customValue));

        if (d->lengthAdjust == KoSvgText::LengthAdjustSpacingAndGlyphs) {
            context.shapeWriter().addAttribute("lengthAdjust", "spacingAndGlyphs");
        }
    }

    KoSvgTextChunkShape *parent = !isRootTextNode() ? dynamic_cast<KoSvgTextChunkShape*>(this->parent()) : 0;
    KoSvgTextProperties parentProperties =
        parent ? parent->textProperties() : KoSvgTextProperties::defaultProperties();

    KoSvgTextProperties ownProperties = textProperties().ownProperties(parentProperties);

    // we write down stroke/fill iff they are different from the parent's value
    if (!isRootTextNode()) {
        if (ownProperties.hasProperty(KoSvgTextProperties::FillId)) {
            SvgStyleWriter::saveSvgFill(this, context);
        }

        if (ownProperties.hasProperty(KoSvgTextProperties::StrokeId)) {
            SvgStyleWriter::saveSvgStroke(this, context);
        }
    }

    QMap<QString, QString> attributes = ownProperties.convertToSvgTextAttributes();
    for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
        context.shapeWriter().addAttribute(it.key().toLatin1().data(), it.value());
    }

    if (layoutInterface()->isTextNode()) {
        context.shapeWriter().addTextNode(d->text);
    } else {
        Q_FOREACH (KoShape *child, this->shapes()) {
            KoSvgTextChunkShape *childText = dynamic_cast<KoSvgTextChunkShape*>(child);
            KIS_SAFE_ASSERT_RECOVER(childText) { continue; }

            childText->saveSvg(context);
        }
    }

    context.shapeWriter().endElement();

    return true;
}


void KoSvgTextChunkShapePrivate::loadContextBasedProperties(SvgGraphicsContext *gc)
{
    properties = gc->textProperties;
    font = gc->font;
    fontFamiliesList = gc->fontFamiliesList;
}

void KoSvgTextChunkShape::resetTextShape()
{
    Q_D(KoSvgTextChunkShape);

    using namespace KoSvgText;

    d->properties = KoSvgTextProperties();
    d->font = QFont();
    d->fontFamiliesList = QStringList();

    d->textLength = AutoValue();
    d->lengthAdjust = LengthAdjustSpacing;

    d->localTransformations.clear();
    d->text.clear();


    // all the subchunks are destroyed!
    // (first detach, then destroy)
    QList<KoShape*> shapesToReset = shapes();
    Q_FOREACH (KoShape *shape, shapesToReset) {
        shape->setParent(0);
        delete shape;
    }
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

    return true;
}

namespace {
bool hasNextSibling(const KoXmlNode &node)
{
    if (!node.nextSibling().isNull()) return true;

    KoXmlNode parentNode = node.parentNode();

    if (!parentNode.isNull() &&
        parentNode.isElement() &&
        parentNode.toElement().tagName() == "tspan") {

        return hasNextSibling(parentNode);
    }

    return false;
}

bool hasPreviousSibling(const KoXmlNode &node)
{
    if (!node.previousSibling().isNull()) return true;

    KoXmlNode parentNode = node.parentNode();

    if (!parentNode.isNull() &&
        parentNode.isElement() &&
        parentNode.toElement().tagName() == "tspan") {

        return hasPreviousSibling(parentNode);
    }

    return false;
}
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

    if (data.startsWith(' ') && !hasPreviousSibling(text)) {
        data.remove(0, 1);
    }

    if (data.endsWith(' ') && !hasNextSibling(text)) {
        data.remove(data.size() - 1, 1);
    }

    if (data == " ") {
        data = "";
    }

    //ENTER_FUNCTION() << text.data() << "-->" << data;

    d->text = data;

    return !data.isEmpty();
}

void KoSvgTextChunkShape::normalizeCharTransformations()
{
    Q_D(KoSvgTextChunkShape);
    d->applyParentCharTransformations(d->localTransformations);
}

void KoSvgTextChunkShape::simplifyFillStrokeInheritance()
{
    if (!isRootTextNode()) {
        KoShape *parentShape = parent();
        KIS_SAFE_ASSERT_RECOVER_RETURN(parentShape);

        QSharedPointer<KoShapeBackground> bg = background();
        QSharedPointer<KoShapeBackground> parentBg = parentShape->background();

        if (!inheritBackground() &&
            ((!bg && !parentBg) ||
             (bg && parentBg &&
              bg->compareTo(parentShape->background().data())))) {

            setInheritBackground(true);
        }

        KoShapeStrokeModelSP stroke = this->stroke();
        KoShapeStrokeModelSP parentStroke= parentShape->stroke();

        if (!inheritStroke() &&
            ((!stroke && !parentStroke) ||
             (stroke && parentStroke &&
              stroke->compareFillTo(parentShape->stroke().data()) &&
              stroke->compareStyleTo(parentShape->stroke().data())))) {

            setInheritStroke(true);
        }
    }


    Q_FOREACH (KoShape *shape, shapes()) {
        KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

        chunkShape->simplifyFillStrokeInheritance();
    }
}

KoSvgTextProperties KoSvgTextChunkShape::textProperties() const
{
    Q_D(const KoSvgTextChunkShape);

    KoSvgTextProperties properties = d->properties;
    properties.setProperty(KoSvgTextProperties::FillId, QVariant::fromValue(KoSvgText::BackgroundProperty(background())));
    properties.setProperty(KoSvgTextProperties::StrokeId, QVariant::fromValue(KoSvgText::StrokeProperty(stroke())));

    return properties;
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

bool KoSvgTextChunkShape::isRootTextNode() const
{
    return false;
}

/**************************************************************************************************/
/*                 KoSvgTextChunkShapePrivate                                                     */
/**************************************************************************************************/

#include "SimpleShapeContainerModel.h"

KoSvgTextChunkShapePrivate::KoSvgTextChunkShapePrivate(KoSvgTextChunkShape *_q)
    : KoShapeContainerPrivate(_q)
{
}

KoSvgTextChunkShapePrivate::KoSvgTextChunkShapePrivate(const KoSvgTextChunkShapePrivate &rhs, KoSvgTextChunkShape *q)
    : KoShapeContainerPrivate(rhs, q),
      properties(rhs.properties),
      font(rhs.font),
      fontFamiliesList(rhs.fontFamiliesList),
      localTransformations(rhs.localTransformations),
      textLength(rhs.textLength),
      lengthAdjust(rhs.lengthAdjust),
      text(rhs.text)
{
    if (rhs.model) {
        SimpleShapeContainerModel *otherModel = dynamic_cast<SimpleShapeContainerModel*>(rhs.model);
        KIS_ASSERT_RECOVER_RETURN(otherModel);
        model = new SimpleShapeContainerModel(*otherModel);
    }
}

KoSvgTextChunkShapePrivate::~KoSvgTextChunkShapePrivate()
{
}

#include <QBrush>
#include <KoColorBackground.h>
#include <KoShapeStroke.h>

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


    QBrush textBrush = Qt::NoBrush;

    if (q->background()) {
        KoColorBackground *colorBackground = dynamic_cast<KoColorBackground*>(q->background().data());
        KIS_SAFE_ASSERT_RECOVER (colorBackground) {
            textBrush = Qt::red;
        }

        if (colorBackground) {
            textBrush = colorBackground->brush();
        }
    }

    format.setForeground(textBrush);

    QPen textPen = Qt::NoPen;

    if (q->stroke()) {
        KoShapeStroke *stroke = dynamic_cast<KoShapeStroke*>(q->stroke().data());
        if (stroke) {
            textPen = stroke->resultLinePen();
        }
    }

    format.setTextOutline(textPen);

    // TODO: avoid const_cast somehow...
    format.setAssociatedShape(const_cast<KoSvgTextChunkShape*>(q));

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
