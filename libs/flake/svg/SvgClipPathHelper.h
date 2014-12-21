/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SVGCLIPPATHHELPER_H
#define SVGCLIPPATHHELPER_H

#include <KoXmlReader.h>

class SvgClipPathHelper
{
public:
    enum Units { UserSpaceOnUse, ObjectBoundingBox };

    SvgClipPathHelper();
    ~SvgClipPathHelper();

    /// Set the clip path units type
    void setClipPathUnits(Units clipPathUnits);
    /// Returns the clip path units type
    Units clipPathUnits() const;

    /// Sets the dom element containing the clip path
    void setContent(const KoXmlElement &content);
    /// Return the clip path element
    KoXmlElement content() const;

private:
    Units m_clipPathUnits;
    KoXmlElement m_content;
};

#endif // SVGCLIPPATHHELPER_H
