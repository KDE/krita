/* This file is part of the KDE project
   Copyright (C) 2005 Johannes Schaub <litb_devel@web.de>
   Copyright (C) 2011 Arjen Hiemstra <ahiemstra@heimr.nl>

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

#ifndef _KOZOOMMODE_H_
#define _KOZOOMMODE_H_

#include <QString>
#include <QFlags>
#include "kritawidgets_export.h"

/**
 * The ZoomMode container
 */
class KRITAWIDGETS_EXPORT KoZoomMode
{
public:
    enum Mode
    {
        ZOOM_CONSTANT = 0,  ///< zoom x %
        ZOOM_WIDTH    = 1,  ///< zoom pagewidth
        ZOOM_PAGE     = 2,  ///< zoom to pagesize
        ZOOM_PIXELS   = 4,   ///< zoom to actual pixels
        ZOOM_TEXT   = 8   ///< zoom to actual pixels
    };

    Q_DECLARE_FLAGS(Modes, Mode)

    /// \param mode the mode name
    /// \return the to Mode converted QString \c mode
    static Mode toMode(const QString& mode);

    /// \return the to QString converted and translated Mode \c mode
    static QString toString(Mode mode);

    /// \param mode the mode name
    /// \return true if \c mode isn't dependent on windowsize
    static bool isConstant(const QString& mode)
    { return toMode(mode) == ZOOM_CONSTANT; }

    /**
     * Return the minimum zoom possible for documents.
     *
     * \return The minimum zoom possible.
     */
    static qreal minimumZoom();
    /**
     * Return the maximum zoom possible for documents.
     *
     * \return The maximum zoom possible.
     */
    static qreal maximumZoom();
    /**
     * Clamp the zoom value so that mimimumZoom <= zoom <= maximumZoom.
     *
     * \param zoom The value to clamp.
     *
     * \return minimumZoom if zoom < minimumZoom, maximumZoom if zoom >
     * maximumZoom, zoom if otherwise.
     */
    static qreal clampZoom(qreal zoom);

    /**
     * Set the minimum zoom possible for documents.
     * 
     * Note that after calling this, any existing KoZoomAction instances
     * should be recreated.
     * 
     * \param zoom The minimum zoom to use.
     */
    static void setMinimumZoom(qreal zoom);
    /**
     * Set the maximum zoom possible for documents.
     * 
     * Note that after calling this, any existing KoZoomAction instances
     * should be recreated.
     *
     * \param zoom The maximum zoom to use.
     */
    static void setMaximumZoom(qreal zoom);
    
private:
    static const char * const modes[];
    static qreal minimumZoomValue;
    static qreal maximumZoomValue;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoZoomMode::Modes)

#endif
