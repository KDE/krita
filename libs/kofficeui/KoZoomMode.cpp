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

#include "KoZoomMode.h"
#include <klocale.h>

const char* KoZoomMode::modes[] = 
{
    I18N_NOOP("%1%"),
    I18N_NOOP("Fit to Width"),
    I18N_NOOP("Fit to Page")
};

QString KoZoomMode::toString(Mode mode)
{
    return i18n(modes[mode]);
}

KoZoomMode::Mode KoZoomMode::toMode(const QString& mode)
{
    if(mode == i18n(modes[ZOOM_WIDTH]))
        return ZOOM_WIDTH;
    else
    if(mode == i18n(modes[ZOOM_PAGE]))
        return ZOOM_PAGE;
    else 
        return ZOOM_CONSTANT;
    // we return ZOOM_CONSTANT else because then we can pass '10%' or '15%'
    // or whatever, it's automatically converted. ZOOM_CONSTANT is 
    // changable, whereas all other zoom modes (non-constants) are normal 
    // text like "Fit to xxx". they let the view grow/shrink according 
    // to windowsize, hence the term 'non-constant'
}
