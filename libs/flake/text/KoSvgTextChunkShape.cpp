/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
v *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoSvgTextChunkShape.h"
#include "KoSvgTextChunkShape_p.h"

#include "KoCssTextUtils.h"
#include "KoSvgText.h"
#include "KoSvgTextProperties.h"

#include <KoPathShape.h>

#include "kis_debug.h"
#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>
#include <SimpleShapeContainerModel.h>

#include <SvgSavingContext.h>
#include <SvgStyleWriter.h>
#include <kis_dom_utils.h>
#include <kis_global.h>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>
#include <commands/KoShapeUngroupCommand.h>

#include <html/HtmlSavingContext.h>

#include <FlakeDebug.h>

#include <QRegularExpression>

namespace {

const QString BIDI_CONTROL_LRE = "\u202a";
const QString BIDI_CONTROL_RLE = "\u202b";
const QString BIDI_CONTROL_PDF = "\u202c";
const QString BIDI_CONTROL_LRO = "\u202d";
const QString BIDI_CONTROL_RLO = "\u202e";
const QString BIDI_CONTROL_LRI = "\u2066";
const QString BIDI_CONTROL_RLI = "\u2067";
const QString BIDI_CONTROL_FSI = "\u2068";
const QString BIDI_CONTROL_PDI = "\u2069";
const QString UNICODE_BIDI_ISOLATE_OVERRIDE_LR_START = "\u2068\u202d";
const QString UNICODE_BIDI_ISOLATE_OVERRIDE_RL_START = "\u2068\u202e";
const QString UNICODE_BIDI_ISOLATE_OVERRIDE_END = "\u202c\u2069";
const QChar ZERO_WIDTH_JOINER = 0x200d;

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

#include <ksharedconfig.h>
#include <kconfiggroup.h>

/**
 * HACK ALERT: this is a function from a private Qt's header qfont_p.h,
 * we don't include the whole header, because it is painful in the
 * environments we don't fully control, e.g. in distribution packages.
 */
Q_GUI_EXPORT int qt_defaultDpi();

namespace {
int forcedDpiForQtFontBugWorkaround() {
    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    int value = cfg.readEntry("forcedDpiForQtFontBugWorkaround", qt_defaultDpi());

    if (value < 0) {
        value = qt_defaultDpi();
    }

    return value;
}


KoSvgTextProperties adjustPropertiesForFontSizeWorkaround(const KoSvgTextProperties &properties)
{
    if (!properties.hasProperty(KoSvgTextProperties::FontSizeId) || !properties.hasProperty(KoSvgTextProperties::FontSizeAdjustId))
        return properties;

    KoSvgTextProperties result = properties;

    const int forcedFontDPI = forcedDpiForQtFontBugWorkaround();

    if (result.hasProperty(KoSvgTextProperties::KraTextVersionId) &&
        result.property(KoSvgTextProperties::KraTextVersionId).toInt() < 2 &&
        forcedFontDPI > 0) {

        qreal fontSize = result.property(KoSvgTextProperties::FontSizeId).toReal();
        fontSize *= qreal(forcedFontDPI) / 72.0;
        result.setProperty(KoSvgTextProperties::FontSizeId, fontSize);
    }
    if (result.hasProperty(KoSvgTextProperties::KraTextVersionId) && result.property(KoSvgTextProperties::KraTextVersionId).toInt() < 3
        && result.hasProperty(KoSvgTextProperties::FontSizeAdjustId)) {
        result.setProperty(KoSvgTextProperties::FontSizeAdjustId, KoSvgText::fromAutoValue(KoSvgText::AutoValue()));
    }

    result.setProperty(KoSvgTextProperties::KraTextVersionId, 3);

    return result;
}

}


struct KoSvgTextChunkShape::Private::LayoutInterface : public KoSvgTextChunkShapeLayoutInterface
{
    LayoutInterface(KoSvgTextChunkShape *_q) : q(_q) {}

    KoSvgText::AutoValue textLength() const override {
        return q->s->textLength;
    }

    KoSvgText::LengthAdjust lengthAdjust() const override {
        return q->s->lengthAdjust;
    }

    int numChars(bool withControls) const override
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->s->text.isEmpty(), 0);

        int result = 0;

        if (!q->shapeCount()) {
            if (withControls) {
                KoSvgText::UnicodeBidi bidi = KoSvgText::UnicodeBidi(q->s->properties.propertyOrDefault(KoSvgTextProperties::UnicodeBidiId).toInt());
                KoSvgText::Direction direction = KoSvgText::Direction(q->s->properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
                // In some circumstances, textTransform can change the length of
                // the text, so we're doing it here too.
                KoSvgText::TextTransformInfo textTransformInfo =
                    q->s->properties.propertyOrDefault(KoSvgTextProperties::TextTransformId).value<KoSvgText::TextTransformInfo>();
                QString lang = q->s->properties.property(KoSvgTextProperties::TextLanguage).toString().toUtf8();
                QVector<QPair<int, int>> positions;

                result = getBidiOpening(direction == KoSvgText::DirectionLeftToRight, bidi).size();
                result += transformText(q->s->text, textTransformInfo, lang, positions).size();
                result += getBidiClosing(bidi).size();
            } else {
                result = q->s->text.size();
            }
        } else {
            Q_FOREACH (KoShape *shape, q->shapes()) {
                KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(chunkShape, 0);
                result += chunkShape->layoutInterface()->numChars(withControls);
            }
        }

        return result;
    }

    int relativeCharPos(KoSvgTextChunkShape *child, int pos) const override {
        QList<KoShape*> childShapes = q->shapes();

        int result = -1;
        int numCharsPassed = 0;

        if (isTextNode() && q == child) {
            result = pos - numCharsPassed;
        } else {
            Q_FOREACH (KoShape *shape, q->shapes()) {
                KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(chunkShape, 0);

                if (chunkShape == child) {
                    result = pos - numCharsPassed;
                    break;
                } else {
                    numCharsPassed += chunkShape->layoutInterface()->numChars(true);
                }

            }
        }

        return result;
    }

    bool isTextNode() const override {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->s->text.isEmpty(), false);
        return !q->shapeCount();
    }

    QString nodeText() const override {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!q->shapeCount() || q->s->text.isEmpty(), 0);
        return !q->shapeCount() ? q->s->text : QString();
    }

    KoSvgText::TextOnPathInfo textOnPathInfo() const override
    {
        return q->s->textPathInfo;
    }

    KoShape *textPath() const override
    {
        return q->s->textPath;
    }

    QVector<KoSvgText::CharTransformation> localCharTransformations() const override {
        return q->s->localTransformations;
    }

    static QString getBidiOpening(bool ltr, KoSvgText::UnicodeBidi bidi)
    {
        using namespace KoSvgText;

        QString result;

        if (ltr) {
            if (bidi == BidiEmbed) {
                result = BIDI_CONTROL_LRE;
            } else if (bidi == BidiOverride) {
                result = BIDI_CONTROL_LRI;
            } else if (bidi == BidiIsolate) {
                result = BIDI_CONTROL_LRO;
            } else if (bidi == BidiIsolateOverride) {
                result = UNICODE_BIDI_ISOLATE_OVERRIDE_LR_START;
            } else if (bidi == BidiPlainText) {
                result = BIDI_CONTROL_FSI;
            }
        } else {
            if (bidi == BidiEmbed) {
                result = BIDI_CONTROL_RLE;
            } else if (bidi == BidiOverride) {
                result = BIDI_CONTROL_RLI;
            } else if (bidi == BidiIsolate) {
                result = BIDI_CONTROL_RLO;
            } else if (bidi == BidiIsolateOverride) {
                result = UNICODE_BIDI_ISOLATE_OVERRIDE_RL_START;
            } else if (bidi == BidiPlainText) {
                result = BIDI_CONTROL_FSI;
            }
        }

        return result;
    }

    static QString getBidiClosing(KoSvgText::UnicodeBidi bidi)
    {
        using namespace KoSvgText;

        QString result;

        if (bidi == BidiEmbed || bidi == BidiOverride) {
            result = BIDI_CONTROL_PDF;
        } else if (bidi == BidiIsolate || bidi == BidiPlainText) {
            result = BIDI_CONTROL_PDI;
        } else if (bidi == BidiIsolateOverride) {
            result = UNICODE_BIDI_ISOLATE_OVERRIDE_END;
        }

        return result;
    }

    static QString transformText(QString text, KoSvgText::TextTransformInfo textTransformInfo, const QString &lang, QVector<QPair<int, int>> &positions)
    {
        if (textTransformInfo.capitals == KoSvgText::TextTransformCapitalize) {
            text = KoCssTextUtils::transformTextCapitalize(text, lang, positions);
        } else if (textTransformInfo.capitals == KoSvgText::TextTransformUppercase) {
            text = KoCssTextUtils::transformTextToUpperCase(text, lang, positions);
        } else if (textTransformInfo.capitals == KoSvgText::TextTransformLowercase) {
            text = KoCssTextUtils::transformTextToLowerCase(text, lang, positions);
        } else {
            positions.clear();
            for (int i = 0; i < text.size(); i++) {
                positions.append(QPair<int, int>(i, i));
            }
        }

        if (textTransformInfo.fullWidth) {
            text = KoCssTextUtils::transformTextFullWidth(text);
        }
        if (textTransformInfo.fullSizeKana) {
            text = KoCssTextUtils::transformTextFullSizeKana(text);
        }
        return text;
    }

    QVector<SubChunk> collectSubChunks(bool textInPath, bool &firstTextInPath) const override
    {
        QVector<SubChunk> result;

        if (q->s->textPath) {
            textInPath = true;
            firstTextInPath = true;
        }

        if (isTextNode()) {
            KoSvgText::TextTransformInfo textTransformInfo =
                q->s->properties.propertyOrDefault(KoSvgTextProperties::TextTransformId).value<KoSvgText::TextTransformInfo>();
            QString lang = q->s->properties.property(KoSvgTextProperties::TextLanguage).toString().toUtf8();
            QVector<QPair<int, int>> positions;
            const QString text = transformText(q->s->text, textTransformInfo, lang, positions);
            const KoSvgText::KoSvgCharChunkFormat format = q->fetchCharFormat();
            QVector<KoSvgText::CharTransformation> transforms = q->s->localTransformations;

            /**
             * Sometimes SVG can contain the X,Y offsets for the pieces of text that
             * do not exist, just skip them.
             */
            if (text.size() <= transforms.size()) {
                transforms.resize(text.size());
            }

            KoSvgText::UnicodeBidi bidi = KoSvgText::UnicodeBidi(q->s->properties.propertyOrDefault(KoSvgTextProperties::UnicodeBidiId).toInt());
            KoSvgText::Direction direction = KoSvgText::Direction(q->s->properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
            const QString bidiOpening = getBidiOpening(direction == KoSvgText::DirectionLeftToRight, bidi);
            const QString bidiClosing = getBidiClosing(bidi);

            if (!bidiOpening.isEmpty()) {
                result << SubChunk(bidiOpening, QString(), format, positions, textInPath, firstTextInPath);
                firstTextInPath = false;
            }

            if (transforms.isEmpty()) {
                result << SubChunk(text, q->s->text, format, positions, textInPath, firstTextInPath);
            } else {
                result << SubChunk(text, q->s->text, format, transforms, positions, textInPath, firstTextInPath);
            }

            if (!bidiClosing.isEmpty()) {
                result << SubChunk(bidiClosing, QString(), format, positions, textInPath, firstTextInPath);
            }
            firstTextInPath = false;

        } else {
            Q_FOREACH (KoShape *shape, q->shapes()) {
                KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
                KIS_SAFE_ASSERT_RECOVER_BREAK(chunkShape);

                result += chunkShape->layoutInterface()->collectSubChunks(textInPath, firstTextInPath);
            }
        }

        if (q->s->textPath) {
            textInPath = false;
            firstTextInPath = false;
        }

        return result;
    }

    void addAssociatedOutline(const QRectF &rect) override {
        KIS_SAFE_ASSERT_RECOVER_RETURN(isTextNode());
        QPainterPath rects;
        rects.addRect(rect);
        rects.addPath(q->s->associatedOutline);
        QPainterPath path;
        path.addRect(rects.boundingRect());

        q->s->associatedOutline = path;
        q->setSize(path.boundingRect().size());

        q->notifyChanged();
        q->shapeChangedPriv(KoShape::SizeChanged);
    }

    void clearAssociatedOutline() override {
        q->s->associatedOutline = QPainterPath();
        q->setSize(QSizeF());

        q->notifyChanged();
        q->shapeChangedPriv(KoShape::SizeChanged);
    }

    void setTextDecorationFontMetrics(KoSvgText::TextDecoration type, qreal offset, qreal width) override
    {
        q->s->textDecorationOffsets.insert(type, offset);
        q->s->textDecorationWidths.insert(type, width);
    }

    qreal getTextDecorationOffset(KoSvgText::TextDecoration type) override
    {
        return q->s->textDecorationOffsets.value(type);
    }

    qreal getTextDecorationWidth(KoSvgText::TextDecoration type) override
    {
        return q->s->textDecorationWidths.value(type);
    }

    void addTextDecoration(KoSvgText::TextDecoration type, QPainterPath path) override
    {
        q->s->textDecorations.insert(type, path);
    }

    void clearTextDecorations() override
    {
        q->s->textDecorations.clear();
    }

    QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations() override
    {
        return q->s->textDecorations;
    }

    void insertText(int start, QString text) override
    {
        if (!q->shapeCount()) {
            if (start >= q->s->text.size()) {
                q->s->text.append(text);
            } else {
                q->s->text.insert(start, text);
            }
        }
    }

    bool isVariationSelector(uint val) {
        // Original set of VS
        if (val == 0xfe00 || (val > 0xfe00 && val <= 0xfe0f)) {
            return true;
        }
        // Extended set VS
        if (val == 0xe0100 || (val > 0xe0100 && val <= 0xe01ef)) {
            return true;
        }
        // Mongolian VS
        if (val == 0x180b || (val > 0x180b && val <= 0x180f)) {
            return true;
        }
        // Emoji skin tones
        if (val == 0x1f3fb || (val > 0x1f3fb && val <= 0x1f3ff)) {
            return true;
        }
        return false;
    }

    bool regionalIndicator(uint val) {
        if (val == 0x1f1e6 || (val > 0x1f1e6 && val <= 0x1f1ff)) {
            return true;
        }
        return false;
    }

    void removeText(int &start, int length) override
    {
        if (isTextNode()) {
            int end = start+length;
            int j = 0;
            int v = 0;
            int lastCharZWJ = 0;
            int lastVS = 0;
            int vsClusterStart = 0;
            int regionalIndicatorCount = 0;
            bool startFound = false;
            bool addToEnd = true;
            Q_FOREACH(const uint i, q->s->text.toUcs4()) {
                v = QChar::requiresSurrogates(i)? 2: 1;
                int index = (j+v) -1;
                bool ZWJ = q->s->text.at(index) == ZERO_WIDTH_JOINER;
                if (isVariationSelector(i)) {
                    lastVS += v;
                } else {
                    lastVS = 0;
                    vsClusterStart = j;
                }
                if (index >= start && !startFound) {
                    startFound = true;
                    if (v > 1) {
                        start = j;
                    }
                    if (regionalIndicatorCount > 0 && regionalIndicator(i)) {
                        start -= regionalIndicatorCount;
                        regionalIndicatorCount = 0;
                    }
                    // Always delete any zero-width-joiners as well.
                    if (ZWJ && index > start) {
                        start = -1;
                    }
                    if (lastCharZWJ > 0) {
                        start -= lastCharZWJ;
                        lastCharZWJ = 0;
                    }
                    // remove any clusters too.
                    if (lastVS > 0) {
                        start = vsClusterStart;
                    }
                }


                if (j >= end && addToEnd) {
                    end = j;
                    addToEnd =  ZWJ || isVariationSelector(i)
                            || (regionalIndicatorCount < 3 && regionalIndicator(i));
                    if (addToEnd) {
                        end += v;
                    }
                }
                j += v;
                lastCharZWJ = ZWJ? lastCharZWJ + v: 0;
                regionalIndicatorCount = regionalIndicator(i)? regionalIndicatorCount + v: 0;
            }
            q->s->text.remove(start, end-start);
        }
    }


    void setTextProperties(KoSvgTextProperties properties) override
    {
        q->s->properties = properties;
    };

private:
    KoSvgTextChunkShape *q;
};

KoSvgTextChunkShape::KoSvgTextChunkShape()
    : KoShapeContainer()
    , d(new Private)
    , s(new SharedData)
{
    d->layoutInterface.reset(new KoSvgTextChunkShape::Private::LayoutInterface(this));
}

KoSvgTextChunkShape::KoSvgTextChunkShape(const KoSvgTextChunkShape &rhs)
    : KoShapeContainer(rhs)
    , d(new Private)
    , s(rhs.s)
{
    if (rhs.model()) {
        SimpleShapeContainerModel *otherModel = dynamic_cast<SimpleShapeContainerModel*>(rhs.model());
        KIS_ASSERT_RECOVER_RETURN(otherModel);
        setModelInit(new SimpleShapeContainerModel(*otherModel));
    }
    d->layoutInterface.reset(new KoSvgTextChunkShape::Private::LayoutInterface(this));
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
    QPainterPath result;
    result.setFillRule(Qt::WindingFill);

    if (d->layoutInterface->isTextNode()) {
        result = s->associatedOutline;
    } else {
        Q_FOREACH (KoShape *shape, shapes()) {
            KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
            KIS_SAFE_ASSERT_RECOVER_BREAK(chunkShape);

            result.addPath(chunkShape->outline());
        }
    }

    return result;
}

void KoSvgTextChunkShape::paintComponent(QPainter &painter) const
{
    Q_UNUSED(painter);
}

bool KoSvgTextChunkShape::saveHtml(HtmlSavingContext &context)
{
    // Should we add a newline? Check for vertical movement if we're using rtl or ltr text
    // XXX: if vertical text, check horizontal movement.
    QVector<qreal> xPos;
    QVector<qreal> yPos;
    QVector<qreal> dxPos;
    QVector<qreal> dyPos;
    QVector<qreal> rotate;

    fillTransforms(&xPos, &yPos, &dxPos, &dyPos, &rotate, s->localTransformations);

    for (int i = 0; i < s->localTransformations.size(); i++) {
        const KoSvgText::CharTransformation &t = s->localTransformations[i];

        appendLazy(&xPos, t.xPos, i, false);
        appendLazy(&yPos, t.yPos, i, false);
        appendLazy(&dxPos, t.dxPos, i);
        appendLazy(&dyPos, t.dyPos, i);
    }

    KoSvgTextChunkShape *parent = !isRootTextNode() ? dynamic_cast<KoSvgTextChunkShape*>(this->parent()) : 0;
    KoSvgTextProperties parentProperties =
        parent ? parent->textProperties() : KoSvgTextProperties::defaultProperties();

    // XXX: we don't save fill, stroke, text length, length adjust or spacing and glyphs.
    KoSvgTextProperties ownProperties = textProperties().ownProperties(parentProperties, isRootTextNode());

    if (isRootTextNode()) {
        context.shapeWriter().startElement("body", false);
        if (layoutInterface()->isTextNode()) {
            context.shapeWriter().startElement("p", false);
        }
        // XXX: Save the style?

    } else if (parent && parent->isRootTextNode()) {
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
        debugFlake << "saveHTML" << this << s->text << xPos << yPos << dxPos << dyPos;
        // After adding all the styling to the <p> element, add the text
        context.shapeWriter().addTextNode(s->text);
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
    bool isTextPath = false;
    if (s->textPath) {
        isTextPath = true;
    }
    if (isRootTextNode()) {
        context.shapeWriter().startElement("text", false);

        if (!context.strippedTextMode()) {
            context.shapeWriter().addAttribute("id", context.getID(this));
            context.shapeWriter().addAttribute("krita:useRichText", s->isRichTextPreferred ? "true" : "false");

            context.shapeWriter().addAttribute("text-rendering", textRenderingString());

            // save the version to distinguish from the buggy Krita version
            // 2: Wrong font-size.
            // 3: Wrong font-size-adjust.
            context.shapeWriter().addAttribute("krita:textVersion", 3);

            SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
            SvgStyleWriter::saveSvgStyle(this, context);
        } else {
            context.shapeWriter().addAttribute("text-rendering", textRenderingString());
            SvgStyleWriter::saveSvgFill(this, context);
            SvgStyleWriter::saveSvgStroke(this, context);
        }
    } else {
        if (isTextPath) {
            context.shapeWriter().startElement("textPath", false);
        } else {
            context.shapeWriter().startElement("tspan", false);
        }
        if (!context.strippedTextMode()) {
            SvgStyleWriter::saveSvgBasicStyle(this, context);
        }
    }

    if (isTextPath) {
        if (s->textPath) {
            // we'll always save as an embedded shape as "path" is an svg 2.0
            // feature.
            QString id = SvgStyleWriter::embedShape(s->textPath, context);
            // inkscape can only read 'xlink:href'
            if (!id.isEmpty()) {
                context.shapeWriter().addAttribute("xlink:href", "#" + id);
            }
        }
        if (s->textPathInfo.startOffset != 0) {
            QString offset = KisDomUtils::toString(s->textPathInfo.startOffset);
            if (s->textPathInfo.startOffsetIsPercentage) {
                offset += "%";
            }
            context.shapeWriter().addAttribute("startOffset", offset);
        }
        if (s->textPathInfo.method != KoSvgText::TextPathAlign) {
            context.shapeWriter().addAttribute("method", KoSvgText::writeTextPathMethod(s->textPathInfo.method));
        }
        if (s->textPathInfo.side != KoSvgText::TextPathSideLeft) {
            context.shapeWriter().addAttribute("side", KoSvgText::writeTextPathSide(s->textPathInfo.side));
        }
        if (s->textPathInfo.spacing != KoSvgText::TextPathAuto) {
            context.shapeWriter().addAttribute("spacing", KoSvgText::writeTextPathSpacing(s->textPathInfo.spacing));
        }
    }

    if (!s->localTransformations.isEmpty()) {

        QVector<qreal> xPos;
        QVector<qreal> yPos;
        QVector<qreal> dxPos;
        QVector<qreal> dyPos;
        QVector<qreal> rotate;

        fillTransforms(&xPos, &yPos, &dxPos, &dyPos, &rotate, s->localTransformations);

        for (int i = 0; i < rotate.size(); i++) {
            rotate[i] = kisRadiansToDegrees(rotate[i]);
        }

        writeTextListAttribute("x", xPos, context.shapeWriter());
        writeTextListAttribute("y", yPos, context.shapeWriter());
        writeTextListAttribute("dx", dxPos, context.shapeWriter());
        writeTextListAttribute("dy", dyPos, context.shapeWriter());
        writeTextListAttribute("rotate", rotate, context.shapeWriter());
    }

    if (!s->textLength.isAuto) {
        context.shapeWriter().addAttribute("textLength", KisDomUtils::toString(s->textLength.customValue));

        if (s->lengthAdjust == KoSvgText::LengthAdjustSpacingAndGlyphs) {
            context.shapeWriter().addAttribute("lengthAdjust", "spacingAndGlyphs");
        }
    }

    KoSvgTextChunkShape *parent = !isRootTextNode() ? dynamic_cast<KoSvgTextChunkShape*>(this->parent()) : 0;
    KoSvgTextProperties parentProperties =
        parent ? parent->textProperties() : KoSvgTextProperties::defaultProperties();

    KoSvgTextProperties ownProperties = textProperties().ownProperties(parentProperties, isRootTextNode());

    ownProperties = adjustPropertiesForFontSizeWorkaround(ownProperties);

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
    QMap<QString, QString> shapeSpecificAttributes = shapeTypeSpecificStyles(context);
    QStringList allowedAttributes = textProperties().supportedXmlAttributes();
    QString styleString;

    for (auto it = shapeSpecificAttributes.constBegin(); it != shapeSpecificAttributes.constEnd(); ++it) {
        styleString.append(it.key().toLatin1().data()).append(": ").append(it.value()).append(";");
    }
    for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
        if (allowedAttributes.contains(it.key())) {
            context.shapeWriter().addAttribute(it.key().toLatin1().data(), it.value());
        } else {
            styleString.append(it.key().toLatin1().data()).append(": ").append(it.value()).append(";");
        }
    }
    if (!styleString.isEmpty()) {
        context.shapeWriter().addAttribute("style", styleString);
    }

    if (layoutInterface()->isTextNode()) {
        context.shapeWriter().addTextNode(s->text);
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

#include <SvgGraphicContext.h>


void KoSvgTextChunkShape::SharedData::loadContextBasedProperties(SvgGraphicsContext *gc)
{
    properties = gc->textProperties;
}

void KoSvgTextChunkShape::resetTextShape()
{
    using namespace KoSvgText;

    s->properties = KoSvgTextProperties();

    s->textLength = AutoValue();
    s->textPath = 0;
    s->lengthAdjust = LengthAdjustSpacing;

    s->localTransformations.clear();
    s->text.clear();


    // all the subchunks are destroyed!
    // (first detach, then destroy)
    QList<KoShape*> shapesToReset = shapes();
    Q_FOREACH (KoShape *shape, shapesToReset) {
        shape->setParent(0);
        delete shape;
    }
}

bool KoSvgTextChunkShape::loadSvg(const QDomElement &e, SvgLoadingContext &context)
{
    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    s->loadContextBasedProperties(gc);

    s->textLength = KoSvgText::parseAutoValueXY(e.attribute("textLength", ""), context, "");
    s->lengthAdjust = KoSvgText::parseLengthAdjust(e.attribute("lengthAdjust", "spacing"));

    QVector<qreal> xPos = parseListAttributeX(e.attribute("x", ""), context);
    QVector<qreal> yPos = parseListAttributeY(e.attribute("y", ""), context);
    QVector<qreal> dxPos = parseListAttributeX(e.attribute("dx", ""), context);
    QVector<qreal> dyPos = parseListAttributeY(e.attribute("dy", ""), context);
    QVector<qreal> rotate = parseListAttributeAngular(e.attribute("rotate", ""), context);

    const int numLocalTransformations =
        std::max({xPos.size(), yPos.size(),
                  dxPos.size(), dyPos.size(),
                  rotate.size()});

    s->localTransformations.resize(numLocalTransformations);
    for (int i = 0; i < numLocalTransformations; i++) {
        if (i < xPos.size()) {
            s->localTransformations[i].xPos = xPos[i];
        }
        if (i < yPos.size()) {
            s->localTransformations[i].yPos = yPos[i];
        }
        if (i < dxPos.size() && dxPos[i] != 0.0) {
            s->localTransformations[i].dxPos = dxPos[i];
        }
        if (i < dyPos.size() && dyPos[i] != 0.0) {
            s->localTransformations[i].dyPos = dyPos[i];
        }
        if (i < rotate.size()) {
            s->localTransformations[i].rotate = rotate[i];
        }
    }

    if (e.tagName() == "textPath") {
        // we'll read the value 'path' later.

        s->textPathInfo.side = KoSvgText::parseTextPathSide(e.attribute("side", "left"));
        s->textPathInfo.method = KoSvgText::parseTextPathMethod(e.attribute("method", "align"));
        s->textPathInfo.spacing = KoSvgText::parseTextPathSpacing(e.attribute("spacing", "auto"));
        // This depends on pathLength;
        if (e.hasAttribute("startOffset")) {
            QString offset = e.attribute("startOffset", "0");
            if (offset.endsWith("%")) {
                s->textPathInfo.startOffset = SvgUtil::parseNumber(offset.left(offset.size() - 1));
                s->textPathInfo.startOffsetIsPercentage = true;
            } else {
                s->textPathInfo.startOffset = SvgUtil::parseUnit(context.currentGC(), offset);
            }
        }
    }

    return true;
}

bool KoSvgTextChunkShape::loadSvgTextNode(const QDomText &text, SvgLoadingContext &context)
{
    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    s->loadContextBasedProperties(gc);

    // In theory, the XML spec requires XML parsers to normalize line endings to
    // LF. However, QXmlInputSource + QXmlSimpleReader do not do this, so we can
    // end up with CR in the text. The SVG spec explicitly calls out that all
    // newlines in SVG are to be represented by a single LF (U+000A) character,
    // so we can replace all CRLF and CR into LF here for simplicity.
    static const QRegularExpression s_regexCrlf(R"==((?:\r\n|\r(?!\n)))==");
    QString content = text.data();
    content.replace(s_regexCrlf, QStringLiteral("\n"));

    s->text = std::move(content);

    return true;
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
    KoSvgTextProperties properties = s->properties;
    properties.setProperty(KoSvgTextProperties::FillId, QVariant::fromValue(KoSvgText::BackgroundProperty(background())));
    properties.setProperty(KoSvgTextProperties::StrokeId, QVariant::fromValue(KoSvgText::StrokeProperty(stroke())));

    return properties;
}

bool KoSvgTextChunkShape::isTextNode() const
{
    return d->layoutInterface->isTextNode();
}

KoSvgTextChunkShapeLayoutInterface *KoSvgTextChunkShape::layoutInterface() const
{
    return d->layoutInterface.data();
}

bool KoSvgTextChunkShape::isRichTextPreferred() const
{
    return isRootTextNode() && s->isRichTextPreferred;
}

void KoSvgTextChunkShape::setRichTextPreferred(bool value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(isRootTextNode());

    s->isRichTextPreferred = value;
}

void KoSvgTextChunkShape::setTextPath(KoShape *path)
{
    s->textPath = path;
}

const KoShape *KoSvgTextChunkShape::textPath()
{
    return s->textPath;
}

bool KoSvgTextChunkShape::isRootTextNode() const
{
    return false;
}

/**************************************************************************************************/
/*                 KoSvgTextChunkShape::Private                                                     */
/**************************************************************************************************/

KoSvgTextChunkShape::SharedData::SharedData()
    : QSharedData()
{
}

KoSvgTextChunkShape::SharedData::SharedData(const SharedData &rhs)
    : QSharedData()
    , properties(rhs.properties)
    , localTransformations(rhs.localTransformations)
    , textPathInfo(rhs.textPathInfo)
    , textLength(rhs.textLength)
    , lengthAdjust(rhs.lengthAdjust)
    , textDecorationOffsets(rhs.textDecorationOffsets)
    , textDecorationWidths(rhs.textDecorationWidths)
    , textDecorations(rhs.textDecorations)
    , text(rhs.text)
    , associatedOutline(rhs.associatedOutline)
    , isRichTextPreferred(rhs.isRichTextPreferred)
{
    if (rhs.textPath) {
        textPath = rhs.textPath->cloneShape();
    }
}

KoSvgTextChunkShape::SharedData::~SharedData()
{
    delete textPath;
}

#include <QBrush>
#include <KoColorBackground.h>
#include <KoShapeStroke.h>

KoSvgText::KoSvgCharChunkFormat KoSvgTextChunkShape::fetchCharFormat() const
{
    KoSvgText::KoSvgCharChunkFormat format;

    const KoSvgTextProperties properties = adjustPropertiesForFontSizeWorkaround(s->properties);

    QFont font(properties.generateFont());

    /**
     * HACK ALERT: Qt has a bug. When requesting font from the font
     * database (in QFontDatabase::load()), Qt scales its size by the
     * current primary display DPI. The only official way to disable
     * that is to assign QTextDocument to QTextLayout, which is not
     * something we would like to do. So we do the hackish way, we
     * just prescale the font with inverted value.
     *
     * This hack changes only the rendering process without touching
     * the way how the text is saved into .kra or .svg. That is nice,
     * but it means it also affects how old files are rendered. To
     * let the user open older files we provide a preference option
     * to enable this scaling again.
     *
     * NOTE:  the hack is not needed for pixel-measured fonts, they
     *        seem to render correctly. Pity we don't use them in
     *        our SVG code (partially because they don't allow
     *        fractional-sized fonts).
     */
    if (font.pointSizeF() > 0) {
        qreal adjustedFontSize = 72.0 / qt_defaultDpi() * font.pointSizeF();
        font.setPointSizeF(adjustedFontSize);
    }

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
    if (kerning.isAuto) {
        format.setFontKerning(true);
    } else {
        format.setFontKerning(false);
        format.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        format.setFontLetterSpacing(format.fontLetterSpacing() + kerning.customValue);
    }


    QBrush textBrush = Qt::NoBrush;

    if (background()) {
        KoColorBackground *colorBackground = dynamic_cast<KoColorBackground*>(background().data());
        if (!colorBackground) {
            qWarning() << "TODO: support gradient and pattern backgrounds for text";
            textBrush = Qt::red;
        }

        if (colorBackground) {
            textBrush = colorBackground->brush();
        }
    }

    format.setForeground(textBrush);

    QPen textPen = Qt::NoPen;

    if (stroke()) {
        KoShapeStroke *stroke = dynamic_cast<KoShapeStroke*>(this->stroke().data());
        if (stroke) {
            textPen = stroke->resultLinePen();
        }
    }

    format.setTextOutline(textPen);

    // TODO: avoid const_cast somehow...
    format.setAssociatedShape(const_cast<KoSvgTextChunkShape*>(this));

    return format;
}
