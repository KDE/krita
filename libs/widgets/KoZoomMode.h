/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005 Johannes Schaub <litb_devel@web.de>
   SPDX-FileCopyrightText: 2011 Arjen Hiemstra <ahiemstra@heimr.nl>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
        ZOOM_HEIGHT   = 16,  ///< zoom pageheight
        ZOOM_PIXELS   = 4   ///< zoom to actual pixels
    };

    Q_DECLARE_FLAGS(Modes, Mode)

    /// \param mode the mode name
    /// \return the to Mode converted QString \c mode
    static Mode toMode(const QString& mode);

    /// \return the to QString converted and translated Mode \c mode
    static QString toString(Mode mode);

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
