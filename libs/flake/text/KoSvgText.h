/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXT_H
#define KOSVGTEXT_H

#include <QVector>
#include <QVariant>
#include <QList>
#include <QFont>
#include <QTextCharFormat>
#include <boost/optional.hpp>
#include <boost/operators.hpp>

#include <QSharedPointer>
#include <KoShapeBackground.h>
#include <KoShapeStrokeModel.h>

#include <KoXmlReaderForward.h>

#include <kritaflake_export.h>

class SvgLoadingContext;
class QDebug;

#include <KoShape.h>
class KoSvgTextChunkShape;

namespace KoSvgText
{
enum WritingMode {
    LeftToRight,
    RightToLeft,
    TopToBottom
};

enum Direction {
    DirectionLeftToRight,
    DirectionRightToLeft
};

enum UnicodeBidi {
    BidiNormal,
    BidiEmbed,
    BidiOverride
};

enum TextAnchor {
    AnchorStart,
    AnchorMiddle,
    AnchorEnd
};

enum DominantBaseline {
    DominantBaselineAuto,
    DominantBaselineUseScript,
    DominantBaselineNoChange,
    DominantBaselineResetSize,
    DominantBaselineIdeographic,
    DominantBaselineAlphabetic,
    DominantBaselineHanging,
    DominantBaselineMathematical,
    DominantBaselineCentral,
    DominantBaselineMiddle,
    DominantBaselineTextAfterEdge,
    DominantBaselineTextBeforeEdge
};

enum AlignmentBaseline {
    AlignmentBaselineAuto,
    AlignmentBaselineDominant,
    AlignmentBaselineIdeographic,
    AlignmentBaselineAlphabetic,
    AlignmentBaselineHanging,
    AlignmentBaselineMathematical,
    AlignmentBaselineCentral,
    AlignmentBaselineMiddle,
    AlignmentBaselineTextAfterEdge,
    AlignmentBaselineTextBeforeEdge
};

enum BaselineShiftMode {
    ShiftNone,
    ShiftSub,
    ShiftSuper,
    ShiftPercentage
    // note: we convert all the <length> values into the relative font values!
};

enum LengthAdjust {
    LengthAdjustSpacing,
    LengthAdjustSpacingAndGlyphs
};

enum TextDecoration {
    DecorationNone = 0x0,
    DecorationUnderline = 0x1,
    DecorationOverline = 0x2,
    DecorationLineThrough = 0x4
};

Q_DECLARE_FLAGS(TextDecorations, TextDecoration)
Q_DECLARE_OPERATORS_FOR_FLAGS(TextDecorations)

/**
 * AutoValue represents the "auto-or-real" values used in SVG
 *
 * Some SVG attributes can be set to either "auto" or some floating point
 * value. E.g. 'kerning' attribute. If its value is "auto", the kerning is
 * defined by the kerning tables of the font. And if its value is a real
 * number, e.g. 0 or 5.5, the font kerning is set to this particular number.
 */
struct AutoValue : public boost::equality_comparable<AutoValue>
{
    AutoValue() {}
    AutoValue(qreal _customValue) : isAuto(false), customValue(_customValue) {}

    bool isAuto = true;
    qreal customValue = 0.0;

    bool operator==(const AutoValue & other) const {
        return isAuto == other.isAuto && (isAuto || customValue == other.customValue);
    }
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::AutoValue &value);

inline QVariant fromAutoValue(const KoSvgText::AutoValue &value) {
    return QVariant::fromValue(value);
}

AutoValue parseAutoValueX(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");
AutoValue parseAutoValueY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");
AutoValue parseAutoValueXY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");
AutoValue parseAutoValueAngular(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");

WritingMode parseWritingMode(const QString &value);
Direction parseDirection(const QString &value);
UnicodeBidi parseUnicodeBidi(const QString &value);
TextAnchor parseTextAnchor(const QString &value);
DominantBaseline parseDominantBaseline(const QString &value);
AlignmentBaseline parseAlignmentBaseline(const QString &value);
BaselineShiftMode parseBaselineShiftMode(const QString &value);
LengthAdjust parseLengthAdjust(const QString &value);

QString writeAutoValue(const AutoValue &value, const QString &autoKeyword = "auto");

QString writeWritingMode(WritingMode value);
QString writeDirection(Direction value);
QString writeUnicodeBidi(UnicodeBidi value);
QString writeTextAnchor(TextAnchor value);
QString writeDominantBaseline(DominantBaseline value);
QString writeAlignmentBaseline(AlignmentBaseline value);
QString writeBaselineShiftMode(BaselineShiftMode value, qreal portion);
QString writeLengthAdjust(LengthAdjust value);

struct CharTransformation : public boost::equality_comparable<CharTransformation>
{
    boost::optional<qreal> xPos;
    boost::optional<qreal> yPos;
    boost::optional<qreal> dxPos;
    boost::optional<qreal> dyPos;
    boost::optional<qreal> rotate;

    void mergeInParentTransformation(const CharTransformation &t);
    bool isNull() const;
    bool startsNewChunk() const;
    bool hasRelativeOffset() const;

    QPointF absolutePos() const;
    QPointF relativeOffset() const;

    bool operator==(const CharTransformation & other) const;
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::CharTransformation &t);

/**
 * @brief The AssociatedShapeWrapper struct is a special shared-pointer-like class
 * to store a safe reference to the associated shape. It implements the shape listener
 * interface and handles 'delete' signal to safely shutdown the link.
 *
 * It is used in KoSvgCharChunkFormat to store a backward link to a shape containing this
 * subchunk of text, so that the layouting engine could notify the shape, where its text
 * is located.
 */
struct AssociatedShapeWrapper : public KoShape::ShapeChangeListener
{
    AssociatedShapeWrapper();
    AssociatedShapeWrapper(KoSvgTextChunkShape *shape);
    AssociatedShapeWrapper(const AssociatedShapeWrapper &rhs);
    AssociatedShapeWrapper& operator=(const AssociatedShapeWrapper &rhs);

    /**
     * @brief isValid shows whether the link to the associated shape is still valid
     * @return true if the link is valid
     */
    bool isValid() const;

    /**
     * @brief addCharacterRect notifies the associated shape that one of its characters
     *                         occupies the location \p rect. The shape is expected to add
     *                         this rect to its outline.
     * @param rect the rectangle associated by the shape
     * @see KoSvgTextChunkShapeLayoutInterface::addAssociatedOutline
     */
    void addCharacterRect(const QRectF &rect);

    void notifyShapeChanged(KoShape::ChangeType type, KoShape *shape) override;

private:
    KoSvgTextChunkShape *m_shape = 0;
};

struct KoSvgCharChunkFormat : public QTextCharFormat
{
    enum SvgCharProperty {
        TextAnchor = UserProperty + 1,
        AssociatedShape
    };

    inline void setTextAnchor(KoSvgText::TextAnchor value) {
        setProperty(TextAnchor, int(value));
    }
    inline KoSvgText::TextAnchor textAnchor() const {
        return KoSvgText::TextAnchor(intProperty(TextAnchor));
    }

    inline Qt::Alignment calculateAlignment() const {
        const KoSvgText::TextAnchor anchor = textAnchor();

        Qt::Alignment result;

        if (layoutDirection() == Qt::LeftToRight) {
            result = anchor == AnchorEnd ? Qt::AlignRight :
                     anchor == AnchorMiddle ? Qt::AlignHCenter :
                     Qt::AlignLeft;
        } else {
            result = anchor == AnchorEnd ? Qt::AlignLeft :
                     anchor == AnchorMiddle ? Qt::AlignHCenter :
                     Qt::AlignRight;
        }

        return result;
    }

    inline void setAssociatedShape(KoSvgTextChunkShape *shape) {
        setProperty(AssociatedShape, QVariant::fromValue(AssociatedShapeWrapper(shape)));
    }

    inline AssociatedShapeWrapper associatedShapeWrapper() const {
        return property(AssociatedShape).value<AssociatedShapeWrapper>();
    }
};

/**
 * @brief BackgroundProperty is a special wrapper around KoShapeBackground for managing it in KoSvgTextProperties
 */
struct BackgroundProperty : public boost::equality_comparable<BackgroundProperty>
{
    BackgroundProperty() {}
    BackgroundProperty(QSharedPointer<KoShapeBackground> p) : property(p) {}

    bool operator==(const BackgroundProperty &rhs) const {
        return (!property && !rhs.property) ||
                (property && rhs.property &&
                 property->compareTo(rhs.property.data()));
    }

    QSharedPointer<KoShapeBackground> property;
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::BackgroundProperty &prop);

/**
 * @brief StrokeProperty is a special wrapper around KoShapeStrokeModel for managing it in KoSvgTextProperties
 */
struct StrokeProperty : public boost::equality_comparable<StrokeProperty>
{
    StrokeProperty() {}
    StrokeProperty(QSharedPointer<KoShapeStrokeModel> p) : property(p) {}

    bool operator==(const StrokeProperty &rhs) const {
        return (!property && !rhs.property) ||
                (property && rhs.property &&
                 property->compareFillTo(rhs.property.data()) && property->compareStyleTo(rhs.property.data()));
    }

    QSharedPointer<KoShapeStrokeModel> property;
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::StrokeProperty &prop);

}

Q_DECLARE_METATYPE(KoSvgText::AutoValue)
Q_DECLARE_METATYPE(KoSvgText::TextDecorations)
Q_DECLARE_METATYPE(KoSvgText::BackgroundProperty)
Q_DECLARE_METATYPE(KoSvgText::StrokeProperty)
Q_DECLARE_METATYPE(KoSvgText::AssociatedShapeWrapper)

#endif // KOSVGTEXT_H
