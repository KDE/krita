/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2002-2003, 2005 Rob Buis <buis@kde.org>
 * SPDX-FileCopyrightText: 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * SPDX-FileCopyrightText: 2005, 2007-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGSTYLEPARSER_H
#define SVGSTYLEPARSER_H

#include "kritaflake_export.h"
#include <QMap>
#include <QGradient>

#include <KoXmlReaderForward.h>

typedef QMap<QString, QString> SvgStyles;

class SvgLoadingContext;
class SvgGraphicsContext;
class QColor;
class QGradient;


class KRITAFLAKE_EXPORT SvgStyleParser
{
public:
    explicit SvgStyleParser(SvgLoadingContext &context);
    ~SvgStyleParser();

    /// Parses specified style attributes
    void parseStyle(const SvgStyles &styles);

    /// Parses font attributes
    void parseFont(const SvgStyles &styles);

    /// Parses a color attribute
    bool parseColor(QColor &, const QString &);

    QPair<qreal, QColor> parseColorStop(const KoXmlElement&, SvgGraphicsContext* context, qreal& previousOffset);

    /// Parses gradient color stops
    void parseColorStops(QGradient *, const KoXmlElement &, SvgGraphicsContext *context, const QGradientStops &defaultStops);

    /// Creates style map from given xml element
    SvgStyles collectStyles(const KoXmlElement &);

    /// Merges two style elements, returning the merged style
    SvgStyles mergeStyles(const SvgStyles &, const SvgStyles &);

    /// Merges two style elements, returning the merged style
    SvgStyles mergeStyles(const KoXmlElement &, const KoXmlElement &);

    SvgStyles parseOneCssStyle(const QString &style, const QStringList &interestingAttributes);
private:

    /// Parses a single style attribute
    void parsePA(SvgGraphicsContext *, const QString &, const QString &);

    /// Returns inherited attribute value for specified element
    QString inheritedAttribute(const QString &attributeName, const KoXmlElement &e);

    class Private;
    Private * const d;
};

#endif // SVGSTYLEPARSER_H
