/* This file is part of the KDE project
 * Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005,2007-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SVGSTYLEPARSER_H
#define SVGSTYLEPARSER_H

#include "flake_export.h"
#include <QMap>

typedef QMap<QString, QString> SvgStyles;

class SvgLoadingContext;
class SvgGraphicsContext;
class KoXmlElement;
class QColor;
class QGradient;

class FLAKE_EXPORT SvgStyleParser
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

    /// Parses gradient color stops
    void parseColorStops(QGradient *, const KoXmlElement &);

    /// Creates style map from given xml element
    SvgStyles collectStyles(const KoXmlElement &);

    /// Merges two style elements, returning the merged style
    SvgStyles mergeStyles(const SvgStyles &, const SvgStyles &);

    /// Merges two style elements, returning the merged style
    SvgStyles mergeStyles(const KoXmlElement &, const KoXmlElement &);

private:

    /// Parses a single style attribute
    void parsePA(SvgGraphicsContext *, const QString &, const QString &);

    /// Returns inherited attribute value for specified element
    QString inheritedAttribute(const QString &attributeName, const KoXmlElement &e);

    class Private;
    Private * const d;
};

#endif // SVGSTYLEPARSER_H
