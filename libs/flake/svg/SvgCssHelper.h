/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SVGCSSHELPER_H
#define SVGCSSHELPER_H

#include <QtCore/QMap>
#include <QtCore/QStringList>

class KoXmlElement;

class SvgCssHelper
{
public:
    SvgCssHelper();
    ~SvgCssHelper();

    /// Parses css style sheet in given xml element
    void parseStylesheet(const KoXmlElement &);

    /**
     * Matches css styles to given xml element and returns them
     * @param element the element to match styles for
     * @return list of matching css styles sorted by priority
     */
    QStringList matchStyles(const KoXmlElement &element) const;

private:
    class Private;
    Private * const d;
};

#endif // SVGCSSHELPER_H
