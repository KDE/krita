/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005 Johannes Schaub <litb_devel@web.de>
   SPDX-FileCopyrightText: 2011 Arjen Hiemstra <ahiemstra@heimr.nl>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KOZOOMMODE_H_
#define _KOZOOMMODE_H_

#include <QFlags>
#include "kritaflake_export.h"

/**
 * The ZoomMode container
 */
class KRITAFLAKE_EXPORT KoZoomMode
{
public:
    enum Mode
    {
        ZOOM_CONSTANT = 0,  ///< zoom x %
        ZOOM_PAGE     = 1,  ///< zoom page
        ZOOM_WIDTH    = 2,  ///< zoom pagewidth
        ZOOM_HEIGHT   = 16,  ///< zoom pageheight
        ZOOM_PIXELS   = 4   ///< zoom to actual pixels TODO: remove!!!
    };

    Q_DECLARE_FLAGS(Modes, Mode)

    /// \param mode the mode name
    /// \return the to Mode converted QString \c mode
    static Mode toMode(const QString& mode);

    /// \return the to QString converted and translated Mode \c mode
    static QString toString(Mode mode);

    /**
     * Generates standard zoom levels for the allowed range of \p minZoom
     * and \p maxZoom
     */
    static QVector<qreal> generateStandardZoomLevels(qreal minZoom, qreal maxZoom);

    /**
     * Find the next zoom level to switch during the zoom-in operation
     */
    static qreal findNextZoom(qreal currentZoom, const QVector<qreal> &zoomLevels);

    /**
     * Find the previous zoom level to switch during the zoom-out operation
     */
    static qreal findPrevZoom(qreal currentZoom, const QVector<qreal> &zoomLevels);

private:
    static const char * const modes[];
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoZoomMode::Modes)

KRITAFLAKE_EXPORT
QDebug operator<<(QDebug dbg, const KoZoomMode::Mode &mode);

#endif
