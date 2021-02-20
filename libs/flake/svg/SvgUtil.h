/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGUTIL_H
#define SVGUTIL_H

#include "kritaflake_export.h"
#include <QRectF>

class QString;
class QTransform;
class QStringList;
class KoXmlWriter;

#include <KoXmlReaderForward.h>

class SvgGraphicsContext;

class KRITAFLAKE_EXPORT SvgUtil
{
public:

    // remove later! pixels *are* user coordinates
    static double fromUserSpace(double value);
    static double toUserSpace(double value);

    static double ptToPx(SvgGraphicsContext *gc, double value);

    /// Converts given point from points to userspace units.
    static QPointF toUserSpace(const QPointF &point);

    /// Converts given rectangle from points to userspace units.
    static QRectF toUserSpace(const QRectF &rect);

    /// Converts given rectangle from points to userspace units.
    static QSizeF toUserSpace(const QSizeF &size);

    /**
     * Parses the given float percentage.
     * @param value the input number containing float percentage (0..1)
     * @return the percentage string (0%..100%), not normalized
     */
    static QString toPercentage(qreal value);

    /**
     * Parses the given string containing a percentage.
     * @param s the input string containing the percentage, float (0..1) or integer (0%..100%)
     * @param ok optional failure indicator
     * @return the float percentage (0..1), not normalized
     */
    static double fromPercentage(QString s, bool *ok=nullptr);

    /**
     * Converts position from objectBoundingBox units to userSpace units.
     */
    static QPointF objectToUserSpace(const QPointF &position, const QRectF &objectBound);

    /**
     * Converts size from objectBoundingBox units to userSpace units.
     */
    static QSizeF objectToUserSpace(const QSizeF &size, const QRectF &objectBound);

    /**
     * Converts position from userSpace units to objectBoundingBox units.
     */
    static QPointF userSpaceToObject(const QPointF &position, const QRectF &objectBound);

    /**
     * Converts size from userSpace units to objectBoundingBox units.
     */
    static QSizeF userSpaceToObject(const QSizeF &size, const QRectF &objectBound);

    /// Converts specified transformation to a string
    static QString transformToString(const QTransform &transform);

    /// Writes a \p transform as an attribute \p name iff the transform is not empty
    static void writeTransformAttributeLazy(const QString &name, const QTransform &transform, KoXmlWriter &shapeWriter);

    /// Parses a viewbox attribute into an rectangle
    static bool parseViewBox(const KoXmlElement &e, const QRectF &elementBounds, QRectF *_viewRect, QTransform *_viewTransform);

    struct PreserveAspectRatioParser;
    static void parseAspectRatio(const PreserveAspectRatioParser &p, const QRectF &elementBounds, const QRectF &viewRect, QTransform *_viewTransform);

    /// Parses a length attribute
    static qreal parseUnit(SvgGraphicsContext *gc, const QString &, bool horiz = false, bool vert = false, const QRectF &bbox = QRectF());

    /// parses a length attribute in x-direction
    static qreal parseUnitX(SvgGraphicsContext *gc, const QString &unit);

    /// parses a length attribute in y-direction
    static qreal parseUnitY(SvgGraphicsContext *gc, const QString &unit);

    /// parses a length attribute in xy-direction
    static qreal parseUnitXY(SvgGraphicsContext *gc, const QString &unit);

    /// parses angle, result in *radians*!
    static qreal parseUnitAngular(SvgGraphicsContext *gc, const QString &unit);

    /// parses the number into parameter number
    static const char * parseNumber(const char *ptr, qreal &number);

    static qreal parseNumber(const QString &string);

    static QString mapExtendedShapeTag(const QString &tagName, const KoXmlElement &element);

    static QStringList simplifyList(const QString &str);

    struct KRITAFLAKE_EXPORT PreserveAspectRatioParser
    {
        PreserveAspectRatioParser(const QString &str);

        enum Alignment {
            Min,
            Middle,
            Max
        };

        bool defer = false;
        Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio;
        Alignment xAlignment = Min;
        Alignment yAlignment = Min;

        QPointF rectAnchorPoint(const QRectF &rc) const;

        QString toString() const;

    private:
        Alignment alignmentFromString(const QString &str) const;
        QString alignmentToString(Alignment alignment) const;
        static qreal alignedValue(qreal min, qreal max, Alignment alignment);
    };
};

#endif // SVGUTIL_H
