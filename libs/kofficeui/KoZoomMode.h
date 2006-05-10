/* This file is part of the KDE project
   Copyright (C) 2005 Johannes Schaub <litb_devel@web.de>

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
#include "koffice_export.h"

/**
 * The ZoomMode container
 */
class KOFFICEUI_EXPORT KoZoomMode
{
public:
    enum Mode
    {
        ZOOM_WIDTH    = 0,  ///< zoom pagewidth
        ZOOM_PAGE     = 1,  ///< zoom to pagesize
        ZOOM_CONSTANT = 2   ///< zoom x %
    };
    
    /// \param mode the mode name
    /// \return the to Mode converted QString \c mode
    static Mode toMode(const QString& mode);
    
    /// \return the to QString converted and translated Mode \c mode
    static QString toString(Mode mode);
    
    /// \param mode the mode name
    /// \return true if \c mode isn't dependent on windowsize
    static bool isConstant(const QString& mode)
    { return toMode(mode) == ZOOM_CONSTANT; }
private:
    static const char * modes[];
};

#endif
