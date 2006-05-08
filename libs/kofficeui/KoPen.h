/* This file is part of the KDE project
   Copyright (C) 2005 Peter Simonsson
   Copyright (C) 2005 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOPEN_H
#define KOPEN_H

#include <qpen.h>

#include "koffice_export.h"

class KoZoomHandler;

/**
 * Pen that handles line widths in points
 */
class KOFFICECORE_EXPORT KoPen : public QPen
{
  public:
    KoPen();
    KoPen(const QColor& _color, double _pointWidth, Qt::PenStyle _style);
    KoPen(const QColor& _color);
    ~KoPen();

    /**
     * @brief Compare pens if they are equal
     *
     * Two pens are equal if they have equal styles, widths and colors.
     *
     * @return true if the pens are equal, false otherwise
     */
    bool operator==( const KoPen &p ) const;

    /**
     * @brief Compare pens if they differ
     *
     * Two pens are different if they have different styles, widths or colors.
     *
     * @return true if the pens are different, false otherwise
     */
    bool operator!=( const KoPen &p ) const;

    /// Set the pen width in points
    void setPointWidth(double width);
    /// KoPen width in points
    double pointWidth() const { return m_pointWidth; }

    /// Returns a zoomed QPen
    QPen zoomedPen(KoZoomHandler* zoomHandler);

  private:
    double m_pointWidth;
};

#endif
